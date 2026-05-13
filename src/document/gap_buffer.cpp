#include "editrr/document/gap_buffer.hpp"

#include <cassert>
#include <cstring>

namespace editrr {

GapBuffer::GapBuffer(size_t initial_capacity) {
  buf_.resize(initial_capacity);
  gap_start_ = 0;
  gap_end_ = initial_capacity;
}

void GapBuffer::erase(size_t pos, size_t count) {
  assert(pos + count <= size());
  move_gap_to(pos);
  gap_end_ += count;
}

void GapBuffer::insert(size_t pos, std::string_view text) {
  move_gap_to(pos);
  if (gap_end_ - gap_start_ < text.size()) {
    grow_gap(text.size());
  }
  for (char c : text) {
    buf_[gap_start_++] = c;
  }
}

void GapBuffer::insert(size_t pos, char c) {
  move_gap_to(pos);
  if (gap_start_ == gap_end_) {
    grow_gap(1);
  }
  buf_[gap_start_++] = c;
}

void GapBuffer::clear() {
  gap_start_ = 0;
  gap_end_ = buf_.size();
}

char GapBuffer::at(size_t pos) const {
  if (pos < gap_start_) {
    return buf_[pos];
  } else {
    return buf_[pos + (gap_end_ - gap_start_)];
  }
}

std::string GapBuffer::to_string() const {
  std::string result;
  result.reserve(size());
  memmove(&result[0], &buf_[0], gap_start_);
  memmove(&result[gap_start_], &buf_[gap_end_], buf_.size() - gap_end_);
  return result;
}

std::string GapBuffer::substr(size_t pos, size_t len) const {
  std::string result;
  result.reserve(len);
  memmove(&result[0], &buf_[0], std::min(len, gap_start_ - pos));
  if (pos + len > gap_start_) {
    memmove(&result[gap_start_ - pos], &buf_[gap_end_],
            std::min(len - (gap_start_ - pos), buf_.size() - gap_end_));
  }
  return result;
}

void GapBuffer::move_gap_to(size_t pos) {
  if (pos < gap_start_) {
    size_t delta = gap_start_ - pos;
    std::memmove(&buf_[gap_end_ - delta], &buf_[pos], delta);
    gap_start_ -= delta;
    gap_end_ -= delta;
  } else if (pos > gap_start_) {
    size_t delta = pos - gap_start_;
    std::memmove(&buf_[gap_start_], &buf_[gap_end_], delta);
    gap_start_ += delta;
    gap_end_ += delta;
  }
}

void GapBuffer::grow_gap(size_t needed) {
  size_t new_capacity = buf_.size() * 2 + needed;
  std::vector<char> new_buf(new_capacity);
  size_t new_gap_end = new_capacity - (buf_.size() - gap_end_);

  std::memcpy(&new_buf[0], &buf_[0], gap_start_);
  std::memcpy(&new_buf[new_gap_end], &buf_[gap_end_], buf_.size() - gap_end_);

  buf_ = std::move(new_buf);
  gap_end_ = new_gap_end;
}
}  // namespace editrr