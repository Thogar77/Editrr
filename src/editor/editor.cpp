#define VERSION "0.0.1"
#include "editor/editor.hpp"

Editor::Editor() {}

void Editor::initialize() {
  cursor.x = 0;
  cursor.y = 0;
  if (get_window_size(height_, width_) == -1) {
    die("Unable to get window size");
  }
}

void Editor::run() {
  initialize();
  enable_raw_mode();

  while (1) {
    refresh_screen();
    process_keypress();
  }
}

void Editor::enable_raw_mode() {
  term.disable({LocalFlag::Echo, LocalFlag::Canonical, LocalFlag::Signals, LocalFlag::Extended});
  term.disable({InputFlag::XonXoff, InputFlag::CarriageReturn, InputFlag::BreakCondition,
                InputFlag::Strip, InputFlag::ParityCheck});
  term.disable({OutputFlag::PostProcessing});
  term.enable({ControlFlag::CharSize8});
}

void Editor::restore() { term.restore(); }

char Editor::read_key() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) term.die("read");
  }
  return c;
}

void Editor::process_keypress() {
  char c = read_key();
  switch (c) {
    case CTRL_KEY('q'):
      clear_screen();
      restore();
      exit(0);
      break;
  }
}

void Editor::draw_rows(std::string& buffer) {
  for (int y = 0; y < height_; y++) {
    if (y == height_ / 3) {
      std::string welcome = "Editrr -- version ";
      welcome += VERSION;
      if (welcome.size() > static_cast<size_t>(width_)) welcome.resize(width_);
      int padding = (width_ - welcome.size()) / 2;
      if (padding) {
        buffer += "~";
        padding--;
      }
      while (padding--) buffer += " ";
      buffer += welcome;
    }
    buffer += "~";
    buffer += "\x1b[K";
    if (y < height_ - 1) buffer += "\r\n";
  }
}

int Editor::get_window_size(int& rows, int& cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    read_key();
    return get_cursor_position(rows, cols);
  } else {
    cols = ws.ws_col;
    rows = ws.ws_row;
    return 0;
  }
}

int Editor::get_cursor_position(int& rows, int& cols) {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", &rows, &cols) != 2) return -1;
  return 0;
}

void Editor::refresh_screen() {
  std::string buffer;
  buffer.reserve(height_ * (width_ + 2));

  buffer += "\x1b[?25l";
  // buffer += "\x1b[2J";
  buffer += "\x1b[H";

  draw_rows(buffer);

  buffer += "\x1b[H";
  buffer += "\x1b[?25h";

  write(STDOUT_FILENO, buffer.c_str(), buffer.size());
}

void Editor::clear_screen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void Editor::die(const std::string error_code) {
  clear_screen();
  term.die(error_code);
}
