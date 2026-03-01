#include "editrr/document/document.hpp"
#include <algorithm>

namespace editrr {

    void Document::rebuild_render(Row& r) {
        r.render.clear();
        r.render.reserve(r.chars.size());
        for (char ch : r.chars) {
            if (ch == '\t') {
                int spaces = TABSTOP - ((int)r.render.size() % TABSTOP);
                r.render.append(spaces, ' ');
            }
            else {
                r.render.push_back(ch);
            }
        }
        r.hl.assign(r.render.size(), Highlight::Normal);
        r.hl_open_comment = false;
    }

    void Document::insert_row(int at, const std::string& s) {
        if (at < 0) at = 0;
        if (at > (int)rows_.size()) at = (int)rows_.size();

        Row r;
        r.chars = s;
        rebuild_render(r);

        rows_.insert(rows_.begin() + at, std::move(r));
        dirty_ = true;
    }

    void Document::delete_row(int at) {
        if (at < 0 || at >= (int)rows_.size()) return;
        rows_.erase(rows_.begin() + at);
        dirty_ = true;
    }

    void Document::row_insert_char(int row_idx, int at, char c) {
        if (row_idx < 0 || row_idx >= (int)rows_.size()) return;
        Row& r = rows_[row_idx];
        if (at < 0 || at >(int)r.chars.size()) at = (int)r.chars.size();
        r.chars.insert(r.chars.begin() + at, c);
        rebuild_render(r);
        dirty_ = true;
    }

    void Document::row_delete_char(int row_idx, int at) {
        if (row_idx < 0 || row_idx >= (int)rows_.size()) return;
        Row& r = rows_[row_idx];
        if (at < 0 || at >= (int)r.chars.size()) return;
        r.chars.erase(r.chars.begin() + at);
        rebuild_render(r);
        dirty_ = true;
    }

    void Document::row_append_string(int row_idx, const std::string& s) {
        if (s.empty()) return;
        if (row_idx < 0 || row_idx >= (int)rows_.size()) return;
        Row& r = rows_[row_idx];
        r.chars += s;
        rebuild_render(r);
        dirty_ = true;
    }

    std::string Document::to_string() const {
        size_t tot = 0;
        for (auto& r : rows_) tot += r.chars.size() + 1;
        std::string out;
        out.reserve(tot);
        for (auto& r : rows_) {
            out += r.chars;
            out += '\n';
        }
        return out;
    }

} // namespace editrr