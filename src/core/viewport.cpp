#include "editrr/core/viewport.hpp"

namespace editrr {

int Viewport::cx_to_rx(std::string_view chars, int cx) const {
  int rx = 0;
  for (int j = 0; j < cx && j < (int)chars.size(); j++) {
    if (chars[j] == '\t')
      rx += (TABSTOP - (rx % TABSTOP));
    else
      rx += 1;
  }
  return rx;
}

int Viewport::rx_to_cx(std::string_view chars, int rx) const {
  int cur_rx = 0;
  int cx = 0;
  for (cx = 0; cx < (int)chars.size(); cx++) {
    if (chars[cx] == '\t')
      cur_rx += (TABSTOP - (cur_rx % TABSTOP));
    else
      cur_rx += 1;
    if (cur_rx > rx) return cx;
  }
  return cx;
}

void Viewport::clamp_cursor(Document& doc, Cursor& cur) const {
  if (cur.y < 0) cur.y = 0;
  if (cur.y > doc.num_rows()) cur.y = doc.num_rows();

  int rowlen = 0;
  if (cur.y < doc.num_rows()) rowlen = (int)doc.row(cur.y).length;

  if (cur.x < 0) cur.x = 0;
  if (cur.x > rowlen) cur.x = rowlen;
}

int Viewport::compute_gutter_width(int num_rows) {
  int digits = 1;
  int max = 10;
  while (num_rows >= max) {
    digits++;
    max *= 10;
  }
  return digits + 2;
}

void Viewport::scroll(Document& doc, Cursor& cur) {
  cur.rx = 0;
  if (cur.y < doc.num_rows()) cur.rx = cx_to_rx(doc.row_text(cur.y), cur.x);

  if (cur.y < rowoff) rowoff = cur.y;
  if (cur.y >= rowoff + height) rowoff = cur.y - height + 1;

  if (cur.rx < coloff) coloff = cur.rx;
  if (cur.rx >= coloff + width) coloff = cur.rx - width + 1;
}

}  // namespace editrr
