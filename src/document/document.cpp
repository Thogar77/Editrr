#include "editrr/document/document.hpp"

#include <algorithm>
#include <stdexcept>

namespace editrr {

void Document::rebuild_render(Row& r, std::string_view text) {
  r.render.clear();
  r.render.reserve(r.length);
  for (char ch : text) {
    if (ch == '\t') {
      int spaces = TABSTOP - ((int)r.render.size() % TABSTOP);
      r.render.append(spaces, ' ');
    } else {
      r.render.push_back(ch);
    }
  }
  r.hl.assign(r.render.size(), Highlight::Normal);
  r.hl_open_comment = false;
}

void Document::insert_row(int at, const std::string& s) {
  if (at < 0) at = 0;
  if (at > (int)rows_.size()) at = (int)rows_.size();
  size_t buf_pos;
  if (rows_.empty()) {
    buf_pos = 0;
  } else if (at == (int)rows_.size()) {
    buf_pos = rows_[at - 1].buf_offset + rows_[at - 1].length + 1;
  } else {
    buf_pos = rows_[at].buf_offset;
  }
  buf_.insert(buf_pos, s);
  buf_.insert(buf_pos + s.size(), '\n');
  Row r;
  r.buf_offset = buf_pos;
  r.length = s.size();
  r.dirty = true;
  rows_.insert(rows_.begin() + at, r);

  update_offsets_from(at + 1);
  dirty_ = true;
}

void Document::delete_row(int at) {
  if (at < 0 || at >= (int)rows_.size()) return;
  size_t buf_pos = rows_[at].buf_offset;
  size_t count = rows_[at].length + 1;
  buf_.erase(buf_pos, count);
  rows_.erase(rows_.begin() + at);
  update_offsets_from(at);
  dirty_ = true;
}

void Document::row_insert_char(int row_idx, int at, char c) {
  if (row_idx < 0 || row_idx >= (int)rows_.size()) return;
  Row& r = rows_[row_idx];
  if (at < 0 || at > r.length) at = r.length;
  size_t buf_pos = r.buf_offset + at;
  buf_.insert(buf_pos, c);
  r.length++;
  r.dirty = true;
  update_offsets_from(row_idx + 1);
  dirty_ = true;
}

void Document::row_delete_char(int row_idx, int at) {
  if (row_idx < 0 || row_idx >= (int)rows_.size()) return;
  if (at < 0 || at > rows_[row_idx].length) return;
  Row& r = rows_[row_idx];
  size_t buf_pos = r.buf_offset + at;
  buf_.erase(buf_pos);
  r.length--;
  r.dirty = true;
  update_offsets_from(row_idx + 1);
  dirty_ = true;
}

void Document::row_append_string(int row_idx, const std::string& s) {
  if (s.empty()) return;
  if (row_idx < 0 || row_idx >= (int)rows_.size()) return;
  Row& r = rows_[row_idx];
  size_t buf_pos = r.buf_offset + r.length;
  buf_.insert(buf_pos, s);
  r.length += s.size();
  r.dirty = true;
  update_offsets_from(row_idx + 1);

  dirty_ = true;
}

void Document::split_row(int row_idx, int at) {
  if (row_idx < 0 || row_idx >= rows_.size()) return;

  std::string right = row_text(row_idx).substr(at);
  size_t buf_pos = rows_[row_idx].buf_offset + at;
  buf_.erase(buf_pos, rows_[row_idx].length - at);

  rows_[row_idx].length = at;
  rows_[row_idx].dirty = true;

  insert_row(row_idx + 1, right);
  dirty_ = true;
}

std::string Document::row_text(int row_idx) const {
  return buf_.substr(rows_[row_idx].buf_offset, rows_[row_idx].length);
}

void Document::load_from_string(const std::string& text) {
  rows_.clear();
  buf_.clear();
  buf_.insert(0, text);
  rebuild_rows_from(0);
  dirty_ = false;
}

void Document::update_offsets_from(int row_idx) {
  if (row_idx < 0 || row_idx >= (int)rows_.size()) return;
  for (int i = row_idx; i < rows_.size(); ++i) {
    if (i == 0) {
      rows_[i].buf_offset = 0;
    } else {
      rows_[i].buf_offset = rows_[i - 1].buf_offset + rows_[i - 1].length + 1;
    }
  }
}

void Document::rebuild_rows_from(size_t buf_pos) {
  if (buf_pos > buf_.size()) return;

  auto it = std::lower_bound(rows_.begin(), rows_.end(), buf_pos,
                             [](const Row& r, size_t pos) { return r.buf_offset < pos; });
  int row_idx = (int)std::distance(rows_.begin(), it);

  size_t current_offset = buf_pos;
  int length = 0;

  for (size_t i = buf_pos; i < buf_.size(); ++i) {
    ++length;
    if (buf_.at(i) == '\n') {
      if (row_idx < (int)rows_.size()) {
        rows_[row_idx].buf_offset = current_offset;
        rows_[row_idx].length = length - 1;
        rows_[row_idx].dirty = true;
      } else {
        Row r;
        r.buf_offset = current_offset;
        r.length = length - 1;
        r.dirty = true;
        rows_.push_back(r);
      }
      current_offset = i + 1;
      length = 0;
      ++row_idx;
    }
  }

  if (length > 0) {
    if (row_idx < (int)rows_.size()) {
      rows_[row_idx].buf_offset = current_offset;
      rows_[row_idx].length = length;
      rows_[row_idx].dirty = true;
    } else {
      Row r;
      r.buf_offset = current_offset;
      r.length = length;
      r.dirty = true;
      rows_.push_back(r);
    }
    ++row_idx;
  }
  if (row_idx < (int)rows_.size()) {
    rows_.erase(rows_.begin() + row_idx, rows_.end());
  }
}
}  // namespace editrr