#include "editrr/app/editor_app.hpp"

#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "editrr/config/app_config.hpp"

namespace editrr {

static int get_window_size(int& rows, int& cols) {
  winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) return -1;
  cols = ws.ws_col;
  rows = ws.ws_row - 2;  // status + message bar
  if (rows < 1) rows = 1;
  return 0;
}

void EditorState::set_status(const char* fmt, ...) {
  char buf[256];
  va_list ap;
  va_start(ap, fmt);
  std::vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  status.text = buf;
  status.time = std::time(nullptr);
}

void EditorState::clamp_cursor() { vp.clamp_cursor(doc, cur); }

void EditorState::on_row_changed(int row_idx) {
  if (row_idx < 0 || row_idx >= doc.num_rows()) return;
  Document::rebuild_render(doc.row(row_idx), doc.row_text(row_idx));

  if (!syntax) return;
  const bool changed = syntax->highlight_row(doc, row_idx);
  if (changed) syntax->highlight_from(doc, row_idx + 1);
}

void EditorState::rehighlight_all() {
  for (int i = 0; i < doc.num_rows(); ++i) Document::rebuild_render(doc.row(i), doc.row_text(i));
  if (!syntax) return;
  syntax->set_filename(doc.filename());
  syntax->highlight_from(doc, 0);
}

void EditorState::move_cursor(KeyCode code) {
  switch (code) {
    case KeyCode::ArrowLeft:
      if (cur.x != 0) {
        cur.x--;
      } else if (cur.y > 0) {
        cur.y--;
        cur.x = (int)doc.row(cur.y).length;
      }
      break;

    case KeyCode::ArrowRight:
      if (cur.y == doc.num_rows()) break;
      if (cur.x < (int)doc.row(cur.y).length) {
        cur.x++;
      } else if (cur.x == (int)doc.row(cur.y).length && cur.y < doc.num_rows() - 1) {
        cur.y++;
        cur.x = 0;
      }
      break;

    case KeyCode::ArrowUp:
      if (cur.y > 0) cur.y--;
      break;

    case KeyCode::ArrowDown:
      if (cur.y < doc.num_rows()) cur.y++;
      break;

    case KeyCode::PageUp:
      cur.y = (cur.y > vp.height) ? (cur.y - vp.height) : 0;
      break;

    case KeyCode::PageDown:
      cur.y = std::min(cur.y + vp.height, doc.num_rows());
      break;

    case KeyCode::Home:
      cur.x = 0;
      break;

    case KeyCode::End:
      cur.x = (cur.y < doc.num_rows()) ? (int)doc.row(cur.y).length : 0;
      break;

    default:
      break;
  }

  clamp_cursor();
}

void EditorState::insert_char(char c) {
  if (cur.y == doc.num_rows()) {
    doc.insert_row(doc.num_rows(), "");
    on_row_changed(doc.num_rows() - 1);
  }

  doc.row_insert_char(cur.y, cur.x, c);
  cur.x++;

  on_row_changed(cur.y);
}

void EditorState::insert_newline() {
  if (cur.y == doc.num_rows()) {
    doc.insert_row(doc.num_rows(), "");
    on_row_changed(doc.num_rows() - 1);
    cur.y++;
    cur.x = 0;
    return;
  }

  if (cur.x == 0) {
    doc.insert_row(cur.y, "");
    on_row_changed(cur.y);
    if (syntax) syntax->highlight_from(doc, cur.y);
  } else {
    doc.split_row(cur.y, cur.x);
    on_row_changed(cur.y);
    on_row_changed(cur.y + 1);
    if (syntax) syntax->highlight_from(doc, cur.y + 1);
  }

  cur.y++;
  cur.x = 0;
}

void EditorState::delete_char(bool delete_key) {
  if (cur.y == doc.num_rows()) return;
  if (doc.num_rows() == 0) return;

  if (!delete_key) {
    // BACKSPACE
    if (cur.x == 0) {
      if (cur.y == 0) return;

      int prev_len = (int)doc.row(cur.y - 1).length;

      // join current into previous
      doc.row_append_string(cur.y - 1, doc.row_text(cur.y));
      doc.delete_row(cur.y);

      cur.y--;
      cur.x = prev_len;

      on_row_changed(cur.y);
      if (syntax) syntax->highlight_from(doc, cur.y + 1);
      return;
    }

    // delete char before cursor
    doc.row_delete_char(cur.y, cur.x - 1);
    cur.x--;

    on_row_changed(cur.y);
    return;
  }

  // DELETE key (delete at cursor)
  Row& row = doc.row(cur.y);
  if (cur.x < (int)row.length) {
    doc.row_delete_char(cur.y, cur.x);
    on_row_changed(cur.y);
  } else {
    // at end -> join with next row
    if (cur.y < doc.num_rows() - 1) {
      doc.row_append_string(cur.y, doc.row_text(cur.y + 1));
      doc.delete_row(cur.y + 1);

      on_row_changed(cur.y);
      if (syntax) syntax->highlight_from(doc, cur.y + 1);
    }
  }
}

void EditorApp::init_terminal(EditorState& state) {
  int rows, cols;
  if (get_window_size(rows, cols) == -1) std::exit(1);
  state.vp.height = rows;
  state.vp.width = cols;
}

void EditorApp::process_key(EditorState& state, EditorServices& svc, const Key& k) {
  if (auto cmd = dispatcher_.map_key_to_command(k)) {
    cmd->execute(state, svc);
  }
}

void EditorApp::run(const std::string& path) {
  EditorState state;
  state.syntax = make_default_highlighter();
  state.set_status("HELP: Ctrl-Q quit | Ctrl-S save | Ctrl-F find");
  state.set_status(("Config path:" + config::AppConfig::get_config_path().string()).c_str());
  EditorServices svc;
  svc.input = &input_;
  svc.renderer = &renderer_;
  svc.file = &file_;
  svc.search = &search_;
  svc.prompt = &prompt_;

  EditorServices srv;
  srv.input = &input_;
  srv.renderer = &renderer_;

  init_terminal(state);
  input_.enable_raw_mode();

  if (!path.empty()) {
    std::string err;
    if (!srv.file->open(state.doc, path, err)) {
      state.set_status("Can't open: %s", err.c_str());
    } else {
      state.rehighlight_all();
    }
  } else {
    state.rehighlight_all();
  }

  while (state.running) {
    int rows, cols;
    if (get_window_size(rows, cols) == -1) std::exit(1);
    state.vp.height = rows;
    state.vp.width = cols;
    renderer_.refresh(state.doc, state.cur, state.vp, state.status, state.doc.filename(),
                      state.doc.dirty());
    Key k = input_.read_key();
    process_key(state, srv, k);
  }

  input_.restore();
}

}  // namespace editrr
