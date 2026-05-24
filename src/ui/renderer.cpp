#include "editrr/ui/renderer.hpp"

#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <ctime>

namespace editrr {

void Renderer::clear_screen() {
  ::write(STDOUT_FILENO, "\x1b[2J", 4);
  ::write(STDOUT_FILENO, "\x1b[H", 3);
}

void Renderer::draw_rows(std::string& out, Document& doc, Viewport& vp) {
  vp.gutter_width = vp.compute_gutter_width(doc.num_rows());

  for (int y = 0; y < vp.height; y++) {
    int filerow = y + vp.rowoff;

    if (filerow < doc.num_rows()) {
      Row& r = doc.row(filerow);

      std::string gutter;
      std::string line_num = std::to_string(filerow + 1);
      int padding = vp.gutter_width - (int)line_num.size() - 2;
      gutter.append(padding, ' ');
      gutter += line_num + " |";
      out += "\x1b[7m";
      out += gutter;
      out += "\x1b[m";
      if (vp.coloff < (int)r.render.size()) {
        int len = std::min((int)r.render.size() - vp.coloff, vp.width - vp.gutter_width);

        Highlight current = Highlight::Normal;
        for (int j = 0; j < len; j++) {
          int idx = vp.coloff + j;
          Highlight hl = (idx >= 0 && idx < (int)r.hl.size()) ? r.hl[idx] : Highlight::Normal;

          if (hl != current) {
            current = hl;
            out += "\x1b[";
            out += hl_to_color(current);
            out += "m";
          }
          out.push_back(r.render[idx]);
        }
        if (current != Highlight::Normal) out += "\x1b[39m";
        r.set_dirty(false);
      }

    } else {
      if (doc.empty() && y == vp.height / 3) {
        std::string welcome = "Editrr -- modular refactor";
        if ((int)welcome.size() > vp.width) welcome.resize(vp.width);
        int padding = (vp.width - (int)welcome.size()) / 2;
        if (padding) {
          out += "~";
          padding--;
        }
        while (padding-- > 0) out += " ";
        out += welcome;
      } else {
        out += "~";
      }
    }
    out += "\x1b[K";
    out += "\r\n";
  }
}

void Renderer::draw_status_bar(std::string& out, Document& doc, Cursor& cur, Viewport& vp,
                               const std::string& filename, bool dirty) {
  out += "\x1b[7m";

  std::string name = filename.empty() ? "[No Name]" : filename;
  std::string left = name + " - " + std::to_string(doc.num_rows()) + " lines";
  if (dirty) left += " (modified)";

  std::string right = std::to_string(cur.y + 1) + "/" + std::to_string(std::max(1, doc.num_rows()));

  std::string bar(vp.width, ' ');
  if ((int)left.size() > vp.width) left.resize(vp.width);
  std::copy(left.begin(), left.end(), bar.begin());

  if ((int)right.size() <= vp.width) {
    std::copy(right.begin(), right.end(), bar.end() - right.size());
  }

  out += bar;
  out += "\x1b[m";
  out += "\r\n";
}

void Renderer::draw_message_bar(std::string& out, Viewport& vp, const StatusMessage& status) {
  out += "\x1b[K";
  if (!status.text.empty()) {
    std::time_t now = std::time(nullptr);
    if (now - status.time < 5) {
      std::string msg = status.text;
      if ((int)msg.size() > vp.width) msg.resize(vp.width);
      out += msg;
    }
  }
}

void Renderer::refresh(Document& doc, Cursor& cur, Viewport& vp, const StatusMessage& status,
                       const std::string& filename, bool dirty) {
  std::string out;
  out.reserve((vp.height + 2) * (vp.width + 16));

  out += "\x1b[?25l";
  out += "\x1b[H";

  vp.scroll(doc, cur);

  draw_rows(out, doc, vp);
  draw_status_bar(out, doc, cur, vp, filename, dirty);
  draw_message_bar(out, vp, status);

  char buf[32];
  std::snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (cur.y - vp.rowoff) + 1,
                (cur.rx - vp.coloff) + 1 + vp.gutter_width);
  out += buf;
  out += "\x1b[?25h";

  ::write(STDOUT_FILENO, out.c_str(), out.size());
}

}  // namespace editrr