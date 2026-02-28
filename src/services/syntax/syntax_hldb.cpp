#include "editrr/services/syntax/syntax_hldb.hpp"

#include <cctype>
#include <cstring>

namespace editrr {

    bool HldbHighlighter::is_quote_(char c) { return c == '"' || c == '\''; }

    bool HldbHighlighter::is_separator_(char c) {
        return c == '\0' || std::isspace((unsigned char)c) || c == ',' || c == '.' ||
            c == '(' || c == ')' || c == '+' || c == '-' || c == '/' || c == '*' ||
            c == '=' || c == '~' || c == '%' || c == '<' || c == '>' || c == '[' ||
            c == ']' || c == ';' || c == '{' || c == '}' || c == ':';
    }

    void HldbHighlighter::set_filename(const std::string& filename) { select_syntax_(filename); }

    void HldbHighlighter::select_syntax_(const std::string& filename) {
        syntax_ = nullptr;
        if (filename.empty()) return;

        for (size_t i = 0; i < HLDB_ENTRIES; i++) {
            const EditorSyntax& s = HLDB[i];

            if (!s.filematch) continue;

            for (size_t j = 0; s.filematch[j]; j++) {
                const char* ext = s.filematch[j];
                const size_t extlen = std::strlen(ext);

                if (filename.size() >= extlen &&
                    filename.compare(filename.size() - extlen, extlen, ext) == 0) {
                    syntax_ = &s;
                    return;
                }
            }
        }
    }

    void HldbHighlighter::highlight_from(Document& doc, int start_row) {
        if (!syntax_) {
            // Brak składni: wyczyść wszystko
            if (start_row < 0) start_row = 0;
            for (int i = start_row; i < doc.num_rows(); ++i) {
                Row& r = doc.row(i);
                r.hl.assign(r.render.size(), Highlight::Normal);
                r.hl_open_comment = false;
            }
            return;
        }

        if (start_row < 0) start_row = 0;
        for (int i = start_row; i < doc.num_rows(); ++i) {
            update_syntax_(doc, i);
        }
    }

    bool HldbHighlighter::highlight_row(Document& doc, int row_idx) {
        if (row_idx < 0 || row_idx >= doc.num_rows()) return false;

        Row& r = doc.row(row_idx);
        const bool old_open = r.hl_open_comment;

        if (!syntax_) {
            r.hl.assign(r.render.size(), Highlight::Normal);
            r.hl_open_comment = false;
            return old_open != r.hl_open_comment;
        }

        update_syntax_(doc, row_idx);
        return r.hl_open_comment != old_open;
    }

    // ====== Twoje update_syntax() przeniesione do serwisu ======
    void HldbHighlighter::update_syntax_(Document& doc, int idx) {
        Row& row = doc.row(idx);

        row.hl.assign(row.render.size(), Highlight::Normal);
        row.hl_open_comment = false;

        if (!syntax_) return;

        const char* scs = syntax_->singleline_comment_start;
        const char* mcs = syntax_->multiline_comment_start;
        const char* mce = syntax_->multiline_comment_end;

        const int scs_len = scs ? (int)std::strlen(scs) : 0;
        const int mcs_len = mcs ? (int)std::strlen(mcs) : 0;
        const int mce_len = mce ? (int)std::strlen(mce) : 0;

        bool in_string = false;
        char string_quote = '\0';

        bool in_comment = (idx > 0) ? doc.row(idx - 1).hl_open_comment : false;

        int prev_sep = 1;
        size_t i = 0;

        while (i < row.render.size()) {
            const char c = row.render[i];

            // ===== single-line comment =====
            if (!in_string && !in_comment && scs_len) {
                if (i + scs_len <= row.render.size() &&
                    std::memcmp(&row.render[i], scs, scs_len) == 0) {
                    for (size_t j = i; j < row.render.size(); j++) row.hl[j] = Highlight::Comment;
                    break;
                }
            }

            // ===== multi-line comments =====
            if (!in_string && mcs_len && mce_len) {
                if (in_comment) {
                    row.hl[i] = Highlight::Comment;
                    if (i + mce_len <= row.render.size() &&
                        std::memcmp(&row.render[i], mce, mce_len) == 0) {
                        for (int j = 0; j < mce_len; j++) row.hl[i + j] = Highlight::Comment;
                        i += mce_len;
                        in_comment = false;
                        prev_sep = 1;
                        continue;
                    }
                    i++;
                    continue;
                }
                else {
                    if (i + mcs_len <= row.render.size() &&
                        std::memcmp(&row.render[i], mcs, mcs_len) == 0) {
                        for (int j = 0; j < mcs_len; j++) row.hl[i + j] = Highlight::Comment;
                        i += mcs_len;
                        in_comment = true;
                        continue;
                    }
                }
            }

            // ===== strings =====
            if (!in_comment) {
                if (in_string) {
                    row.hl[i] = Highlight::String;

                    // escape
                    if (c == '\\' && i + 1 < row.render.size()) {
                        row.hl[i + 1] = Highlight::String;
                        i += 2;
                        continue;
                    }

                    if (c == string_quote) {
                        in_string = false;
                        string_quote = '\0';
                    }

                    i++;
                    prev_sep = 1;
                    continue;
                }
                else {
                    if (is_quote_(c)) {
                        in_string = true;
                        string_quote = c;
                        row.hl[i] = Highlight::String;
                        i++;
                        continue;
                    }
                }
            }

            // ===== numbers =====
            if (std::isdigit((unsigned char)c) &&
                (prev_sep || (i > 0 && row.hl[i - 1] == Highlight::Number))) {
                row.hl[i] = Highlight::Number;
                i++;
                prev_sep = 0;
                continue;
            }

            // ===== keywords =====
            if (prev_sep) {
                bool matched_keyword = false;

                const char** keywords = syntax_->keywords;
                for (size_t j = 0; keywords && keywords[j]; j++) {
                    const char* kw = keywords[j];
                    size_t kwlen = std::strlen(kw);
                    Highlight kw_hl = Highlight::Keyword1;

                    if (kwlen > 0 && kw[kwlen - 1] == '|') {
                        kw_hl = Highlight::Keyword2;
                        kwlen--;
                    }
                    if (kwlen == 0) continue;

                    if (i + kwlen <= row.render.size() &&
                        std::memcmp(&row.render[i], kw, kwlen) == 0 &&
                        is_separator_(i + kwlen < row.render.size() ? row.render[i + kwlen] : '\0')) {
                        for (size_t k = 0; k < kwlen; k++) row.hl[i + k] = kw_hl;

                        i += kwlen;
                        prev_sep = 0;
                        matched_keyword = true;
                        break;
                    }
                }

                if (matched_keyword) continue;
            }

            // ===== default =====
            prev_sep = is_separator_(c);
            i++;
        }

        row.hl_open_comment = in_comment;
    }

} // namespace editrr