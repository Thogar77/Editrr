#include "editrr/core/viewport.hpp"

namespace editrr {

    int Viewport::cx_to_rx(const Row& row, int cx) const {
        int rx = 0;
        for (int j = 0; j < cx && j < (int)row.chars.size(); j++) {
            if (row.chars[j] == '\t') rx += (TABSTOP - (rx % TABSTOP));
            else rx += 1;
        }
        return rx;
    }

    int Viewport::rx_to_cx(const Row& row, int rx) const {
        int cur_rx = 0;
        int cx = 0;
        for (cx = 0; cx < (int)row.chars.size(); cx++) {
            if (row.chars[cx] == '\t') cur_rx += (TABSTOP - (cur_rx % TABSTOP));
            else cur_rx += 1;
            if (cur_rx > rx) return cx;
        }
        return cx;
    }

    void Viewport::clamp_cursor(Document& doc, Cursor& cur) const {
        if (cur.y < 0) cur.y = 0;
        if (cur.y > doc.num_rows()) cur.y = doc.num_rows();

        int rowlen = 0;
        if (cur.y < doc.num_rows()) rowlen = (int)doc.row(cur.y).chars.size();

        if (cur.x < 0) cur.x = 0;
        if (cur.x > rowlen) cur.x = rowlen;
    }

    void Viewport::scroll(Document& doc, Cursor& cur) {
        cur.rx = 0;
        if (cur.y < doc.num_rows()) cur.rx = cx_to_rx(doc.row(cur.y), cur.x);

        // vertical
        if (cur.y < rowoff) rowoff = cur.y;
        if (cur.y >= rowoff + height) rowoff = cur.y - height + 1;

        // horizontal
        if (cur.rx < coloff) coloff = cur.rx;
        if (cur.rx >= coloff + width) coloff = cur.rx - width + 1;
    }

} // namespace editrr