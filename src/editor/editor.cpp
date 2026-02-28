#define VERSION "0.0.1"
#include "editor/editor.hpp"
namespace editor {
  Editor::Editor() {}

  void Editor::initialize() {
    cursor.x = 0;
    cursor.y = 0;
    cursor.rx = 0;

    rowoff_ = 0;
    coloff_ = 0;

    buffer_.rows.clear();

    set_status_message("HELP: Ctrl-Q = quit");

    if (get_window_size(height_, width_) == -1) die("Unable to get window size");

  }

  void Editor::run(const std::string& file_to_open) {
    initialize();
    enable_raw_mode();
    if (!file_to_open.empty()) {
      open_file(file_to_open);
    }
    while (1) {
      refresh_screen();
      process_keypress();
    }
  }

  void Editor::select_syntax_highlight_() {
    syntax_ = nullptr;
    if (filename_.empty()) return;

    for (size_t i = 0; i < HLDB_ENTRIES; i++) {
      const EditorSyntax& s = HLDB[i];
      for (size_t j = 0; s.filematch[j]; j++) {
        const char* ext = s.filematch[j];
        size_t extlen = std::strlen(ext);

        if (filename_.size() >= extlen &&
          filename_.compare(filename_.size() - extlen, extlen, ext) == 0) {
          syntax_ = &s;
          update_syntax_from_(0); // policz wszystko od nowa
          return;
        }
      }
    }
  }
  void Editor::update_row(int idx) {
    Row& row = buffer_.rows[idx];

    row.render.clear();
    row.render.reserve(row.chars.size());

    for (char ch : row.chars) {
      if (ch == '\t') {
        int spaces = TABSTOP - ((int)row.render.size() % TABSTOP);
        row.render.append(spaces, ' ');
      }
      else {
        row.render.push_back(ch);
      }
    }

    bool old_open = row.hl_open_comment;
    update_syntax(idx);

    if (row.hl_open_comment != old_open) {
      update_syntax_from_(idx + 1);
    }
  }
  int Editor::cx_to_rx_(const Row& row, int cx) const {
    int rx = 0;
    for (int j = 0; j < cx && j < (int)row.chars.size(); j++) {
      if (row.chars[j] == '\t') {
        rx += (TABSTOP - (rx % TABSTOP));
      }
      else {
        rx += 1;
      }
    }
    return rx;
  }

  void Editor::enable_raw_mode() {
    term.disable({ LocalFlag::Echo, LocalFlag::Canonical, LocalFlag::Signals, LocalFlag::Extended });
    term.disable({ InputFlag::XonXoff, InputFlag::CarriageReturn, InputFlag::BreakCondition,
                  InputFlag::Strip, InputFlag::ParityCheck });
    term.disable({ OutputFlag::PostProcessing });
    term.enable({ ControlFlag::CharSize8 });
  }

  void Editor::restore() { term.restore(); }

  int Editor::read_key() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
      if (nread == -1 && errno != EAGAIN) term.die("read");
    }

    if (c == '\x1b') {
      char seq[3];
      if (read(STDIN_FILENO, &seq[0], 1) != 1) return editor::KEY_ESC;
      if (read(STDIN_FILENO, &seq[1], 1) != 1) return editor::KEY_ESC;

      if (seq[0] == '[') {
        if (seq[1] >= '0' && seq[1] <= '9') {
          if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
          if (seq[2] == '~') {
            switch (seq[1]) {
            case '1': return HOME_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
            }
          }
        }
        else {
          switch (seq[1]) {
          case 'A': return editor::ARROW_UP;
          case 'B': return editor::ARROW_DOWN;
          case 'C': return editor::ARROW_RIGHT;
          case 'D': return editor::ARROW_LEFT;
          case 'H': return HOME_KEY;
          case 'F': return END_KEY;
          }
        }
      }
      else if (seq[0] == 'O') {
        switch (seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
        }
      }

      return editor::KEY_ESC;
    }

    return static_cast<unsigned char>(c);
  }

  void Editor::process_keypress() {
    static int quit_times = QUIT_TIMES;

    const int c = read_key();

    switch (c) {
    case CTRL_KEY('q'):
      if (dirty_ && quit_times > 0) {
        set_status_message(
          "WARNING!!! File has unsaved changes. Press Ctrl-Q %d more times to quit.",
          quit_times
        );
        quit_times--;
        return;
      }
      clear_screen();
      restore();
      std::exit(0);

    case CTRL_KEY('f'): {
      find();
      break;
    }
    case CTRL_KEY('s'):
      save_to_disk();
      break;
    case ENTER:
      insert_newline();
      break;
    case BACKSPACE:
    case DELETE_KEY:
      del_char();
      break;
    case KEY_ESC:
      break;

    default:
      if (auto it = config_.keymap.find(c); it != config_.keymap.end()) {
        dispatch_action(it->second);
      }
      else if (c >= 32 && c <= 126) {
        insert_char(c);
      }
      break;
    }

    // jeśli user nacisnął coś innego niż Ctrl-Q, resetujemy licznik
    quit_times = QUIT_TIMES;
  }

  void Editor::row_append_string(Row& row, const std::string& s) {
    if (s.empty()) return;
    row.chars += s;

    const int idx = (int)(&row - &buffer_.rows[0]); // tylko jeśli row pochodzi z buffer_.rows
    update_row(idx);

    dirty_ = true;
  }

  void Editor::del_row(int at) {
    if (at < 0 || at >= (int)buffer_.rows.size()) return;
    buffer_.rows.erase(buffer_.rows.begin() + at);
    dirty_ = true;
  }


  void Editor::draw_rows(std::string& out) {
    for (int y = 0; y < height_; y++) {
      const int filerow = y + rowoff_;

      if (filerow < (int)buffer_.rows.size()) {
        const std::string& line = buffer_.rows[filerow].render;

        const Row& r = buffer_.rows[filerow];

        if (coloff_ < (int)r.render.size()) {
          const int len = std::min((int)r.render.size() - coloff_, width_);

          Highlight current = Highlight::Normal;

          for (int j = 0; j < len; j++) {
            const int idx = coloff_ + j;

            Highlight hl = (idx >= 0 && idx < (int)r.hl.size()) ? r.hl[idx] : Highlight::Normal;

            if (hl != current) {
              current = hl;
              out += "\x1b[";
              out += hl_to_color(current);
              out += "m";
            }

            out.push_back(r.render[idx]);
          }

          // reset na koniec linii
          if (current != Highlight::Normal) out += "\x1b[39m";
        }
      }
      else {
        if (buffer_.rows.empty() && y == height_ / 3) {
          std::string welcome = "Editrr -- version " VERSION;
          if ((int)welcome.size() > width_) welcome.resize(width_);

          int padding = (width_ - (int)welcome.size()) / 2;
          if (padding) { out += "~"; padding--; }
          while (padding-- > 0) out += " ";
          out += welcome;
        }
        else {
          out += "~";
        }
      }

      out += "\x1b[K";
      // if (y < height_ - 1) 
      out += "\r\n";
    }
  }
  void Editor::update_syntax(int idx) {
    Row& row = buffer_.rows[idx];

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

    bool in_comment = (idx > 0) ? buffer_.rows[idx - 1].hl_open_comment : false;

    int prev_sep = 1;
    size_t i = 0;

    while (i < row.render.size()) {
      char c = row.render[i];

      // ===== single-line comment (// ...) =====
      if (!in_string && !in_comment && scs_len) {
        if (i + scs_len <= row.render.size() &&
          std::memcmp(&row.render[i], scs, scs_len) == 0) {
          for (size_t j = i; j < row.render.size(); j++) row.hl[j] = Highlight::Comment;
          break;
        }
      }

      // ===== multi-line comments (/* ... */) =====
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

      // ===== strings ("..." / '...') =====
      if (!in_comment) {
        if (in_string) {
          row.hl[i] = Highlight::String;

          // escape: \" albo \'
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
          if (is_quote(c)) {
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
            is_separator(i + kwlen < row.render.size() ? row.render[i + kwlen] : '\0')) {

            for (size_t k = 0; k < kwlen; k++) row.hl[i + k] = kw_hl;

            i += kwlen;
            prev_sep = 0;

            matched_keyword = true;
            break;
          }
        }

        if (matched_keyword) continue;  // <-- kluczowe: nie robimy i++ poniżej
      }

      // ===== default =====
      prev_sep = is_separator((unsigned char)c);
      i++;
    }

    row.hl_open_comment = in_comment;
  }
  void Editor::find() {
    const int saved_cx = cursor.x;
    const int saved_cy = cursor.y;
    const int saved_coloff = coloff_;
    const int saved_rowoff = rowoff_;

    auto query = prompt(
      "Search: %s (ESC to cancel, Arrows to navigate)",
      [this](const std::string& q, int key) {
        this->find_callback_(q, key);
      }
    );

    if (!query) {
      cursor.x = saved_cx;
      cursor.y = saved_cy;
      coloff_ = saved_coloff;
      rowoff_ = saved_rowoff;
    }
  }
  void Editor::find_callback_(const std::string& query, int key) {
    if (key == ENTER || key == KEY_ESC) {
      find_last_match_ = -1;
      find_direction_ = 1;
      return;
    }

    if (key == editor::ARROW_RIGHT || key == editor::ARROW_DOWN) {
      find_direction_ = 1;
    }
    else if (key == editor::ARROW_LEFT || key == editor::ARROW_UP) {
      find_direction_ = -1;
    }
    else {
      find_last_match_ = -1;
      find_direction_ = 1;
    }

    if (query.empty() || buffer_.rows.empty()) return;

    int current = find_last_match_;
    for (int i = 0; i < (int)buffer_.rows.size(); i++) {
      current += find_direction_;

      if (current == -1) current = (int)buffer_.rows.size() - 1;
      if (current == (int)buffer_.rows.size()) current = 0;

      const Row& row = buffer_.rows[current];
      const size_t pos = row.render.find(query);
      if (pos != std::string::npos) {
        find_last_match_ = current;

        cursor.y = current;
        cursor.x = rx_to_cx_(row, (int)pos);

        rowoff_ = (int)buffer_.rows.size();
        return;
      }
    }
  }
  void Editor::scroll() {
    cursor.rx = 0;
    if (cursor.y < (int)buffer_.rows.size()) {
      cursor.rx = cx_to_rx_(buffer_.rows[cursor.y], cursor.x);
    }

    // vertical
    if (cursor.y < rowoff_) rowoff_ = cursor.y;
    if (cursor.y >= rowoff_ + height_) rowoff_ = cursor.y - height_ + 1;

    // horizontal (po rx)
    if (cursor.rx < coloff_) coloff_ = cursor.rx;
    if (cursor.rx >= coloff_ + width_) coloff_ = cursor.rx - width_ + 1;
  }

  void Editor::open_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) die("open_file");

    buffer_.rows.clear();
    filename_ = path;

    std::string line;
    while (std::getline(in, line)) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      buffer_.rows.push_back(Row{ line, {}, {}, false });
    }

    for (int i = 0; i < (int)buffer_.rows.size(); i++) {
      Row& r = buffer_.rows[i];
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
      r.hl.clear();
      r.hl_open_comment = false;
    }

    select_syntax_highlight_();

    cursor.x = 0;
    cursor.y = 0;
    rowoff_ = 0;
    coloff_ = 0;
  }

  void Editor::row_del_char(Row& row, int at) {
    if (at < 0 || at >= (int)row.chars.size()) return;

    row.chars.erase(row.chars.begin() + at);
    update_row(row);
    dirty_ = true;
  }
  void Editor::del_char() {
    if (cursor.y == (int)buffer_.rows.size()) return;

    if (cursor.x == 0) {
      if (cursor.y == 0) return;

      int prev_len = (int)buffer_.rows[cursor.y - 1].chars.size();

      row_append_string(buffer_.rows[cursor.y - 1], buffer_.rows[cursor.y].chars);

      del_row(cursor.y);

      cursor.y--;
      cursor.x = prev_len;
      return;
    }

    // zwykły backspace w środku linii
    row_del_char(buffer_.rows[cursor.y], cursor.x - 1);
    cursor.x--;
  }

  void Editor::update_syntax_from_(int start) {
    if (!syntax_) return;
    if (start < 0) start = 0;

    for (int i = start; i < (int)buffer_.rows.size(); ++i) {
      update_syntax(i);
    }
  }

  void Editor::clear_saved_hl_() {
    if (saved_hl_line_ != -1) {
      Row& r = buffer_.rows[saved_hl_line_];
      if (saved_hl_.size() == r.hl.size()) r.hl = saved_hl_;
      saved_hl_.clear();
      saved_hl_line_ = -1;
    }
  }

  std::optional<std::string> Editor::prompt(
    const char* prompt_fmt,
    const std::function<void(const std::string&, int)>& callback) {

    std::string buf;
    buf.reserve(128);

    while (true) {
      set_status_message(prompt_fmt, buf.c_str());
      refresh_screen();

      const int c = read_key();

      if (c == BACKSPACE || c == DELETE_KEY || c == CTRL_KEY('h')) {
        if (!buf.empty()) buf.pop_back();
      }
      else if (c == KEY_ESC) {
        set_status_message("");
        if (callback) callback(buf, c);
        return std::nullopt;
      }
      else if (c == ENTER) {
        set_status_message("");
        if (callback) callback(buf, c);
        return buf;
      }
      else if (!iscntrl(c) && c < 128) {
        buf.push_back((char)c);
      }

      if (callback) callback(buf, c);
    }
  }
  std::optional<std::string> Editor::prompt(const char* prompt_fmt) {
    std::string buf;
    buf.reserve(128);

    while (true) {
      set_status_message(prompt_fmt, buf.c_str());
      refresh_screen();

      const int c = read_key();

      // Backspace / Delete / Ctrl-H
      if (c == BACKSPACE || c == DELETE_KEY || c == CTRL_KEY('h')) {
        if (!buf.empty()) buf.pop_back();
        continue;
      }

      // ESC -> cancel
      if (c == KEY_ESC) {
        set_status_message("");
        return std::nullopt;
      }

      // Enter -> accept (jak niepuste)
      if (c == ENTER) {
        if (!buf.empty()) {
          set_status_message("");
          return buf;
        }
        continue;
      }

      // zwykłe znaki drukowalne
      if (c >= 32 && c <= 126) {
        buf.push_back((char)c);
        continue;
      }

    }
  }
  void Editor::draw_status_bar(std::string& out) {
    out += "\x1b[7m"; // inverted colors

    std::string name = filename_.empty() ? "[No Name]" : filename_;
    std::string left = name + " - " + std::to_string(buffer_.rows.size()) + " lines";
    if (dirty_) left += " (modified)";
    std::string right = std::to_string(cursor.y + 1) + "/" +
      std::to_string(std::max<size_t>(1, buffer_.rows.size()));

    std::string bar(width_, ' ');

    if ((int)left.size() > width_) left.resize(width_);
    std::copy(left.begin(), left.end(), bar.begin());

    if ((int)right.size() <= width_) {
      std::copy(right.begin(), right.end(), bar.end() - right.size());
    }

    out += bar;
    out += "\x1b[m";
    out += "\r\n";
  }
  int Editor::get_window_size(int& rows, int& cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
      if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
      read_key();
      return get_cursor_position(rows, cols);
    }
    else {
      cols = ws.ws_col;
      rows = ws.ws_row - 2;
      if (rows < 1) rows = 1;
      return 0;
    }
  }
  void Editor::set_status_message(const char* fmt, ...) {
    char buf[256];

    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    statusmsg_ = buf;
    statusmsg_time_ = std::time(nullptr);
  }
  void Editor::draw_message_bar(std::string& out) {
    out += "\x1b[K"; // clear line

    if (!statusmsg_.empty()) {
      const std::time_t now = std::time(nullptr);
      if (now - statusmsg_time_ < 5) {
        std::string msg = statusmsg_;
        if ((int)msg.size() > width_) msg.resize(width_);
        out += msg;
      }
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
    std::string out;
    out.reserve((height_ + 2) * (width_ + 2));

    out += "\x1b[?25l"; // hide cursor
    out += "\x1b[H";    // go home

    scroll();
    draw_rows(out);
    draw_status_bar(out);
    draw_message_bar(out);

    char buf[32];
    std::snprintf(buf, sizeof(buf), "\x1b[%d;%dH",
      (cursor.y - rowoff_) + 1,
      (cursor.rx - coloff_) + 1);

    out += buf;
    out += "\x1b[?25h"; // show cursor

    write(STDOUT_FILENO, out.c_str(), out.size());
  }
  void Editor::row_insert_char(Row& row, int at, char c) {
    if (at < 0 || at >(int)row.chars.size()) {
      at = row.chars.size();
    }

    row.chars.insert(row.chars.begin() + at, c);
    update_row(row);
    dirty_ = true;
  }
  void Editor::update_row(Row& row) {
    if (buffer_.rows.empty()) return;
    const ptrdiff_t idx = &row - &buffer_.rows[0];
    if (idx < 0 || idx >= (ptrdiff_t)buffer_.rows.size()) return;
    update_row((int)idx);
  }

  void Editor::insert_row(int at, const std::string& s) {
    if (at < 0) at = 0;
    if (at > (int)buffer_.rows.size()) at = (int)buffer_.rows.size();

    buffer_.rows.insert(buffer_.rows.begin() + at, Row{ s, {}, {}, false });
    update_row(at);
    dirty_ = true;
  }

  void Editor::insert_newline() {
    if (cursor.y == (int)buffer_.rows.size()) {
      insert_row((int)buffer_.rows.size(), "");
      cursor.y++;
      cursor.x = 0;
      return;
    }

    if (cursor.x == 0) {
      insert_row(cursor.y, "");
    }
    else {
      Row& row = buffer_.rows[cursor.y];

      std::string right = row.chars.substr(cursor.x);
      row.chars.erase(cursor.x);
      update_row(row);
      dirty_ = true;

      insert_row(cursor.y + 1, right);
    }

    cursor.y++;
    cursor.x = 0;
  }

  void Editor::insert_char(int c) {
    if (cursor.y == (int)buffer_.rows.size()) {
      buffer_.rows.push_back(Row{ "", {}, {}, false });
      update_row((int)buffer_.rows.size() - 1);
    }

    row_insert_char(buffer_.rows[cursor.y], cursor.x, (char)c);
    cursor.x++;
  }
  void Editor::save_to_disk() {
    if (filename_.empty()) {
      auto name = prompt("Save as: %s (ESC to cancel)");
      if (!name) {
        set_status_message("Save aborted");
        return;
      }
      filename_ = *name;
      select_syntax_highlight_();
    }

    const std::string data = rows_to_string();

    int fd = ::open(filename_.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
      set_status_message("Can't save! I/O error: %s", std::strerror(errno));
      return;
    }

    if (::ftruncate(fd, (off_t)data.size()) == -1) {
      ::close(fd);
      set_status_message("Can't save! I/O error: %s", std::strerror(errno));
      return;
    }

    const ssize_t written = ::write(fd, data.data(), data.size());
    ::close(fd);

    if (written == (ssize_t)data.size()) {
      dirty_ = false;
      set_status_message("%zu bytes written to disk", data.size());
    }
    else {
      set_status_message("Can't save! I/O error: %s", std::strerror(errno));
    }
  }

  int Editor::rx_to_cx_(const Row& row, int rx) const {
    int cur_rx = 0;
    int cx = 0;

    for (cx = 0; cx < (int)row.chars.size(); cx++) {
      if (row.chars[cx] == '\t') {
        cur_rx += (TABSTOP - (cur_rx % TABSTOP));
      }
      else {
        cur_rx += 1;
      }
      if (cur_rx > rx) return cx;
    }
    return cx;
  }
  std::string Editor::rows_to_string() const {
    // policz rozmiar (opcjonalne, ale fajne dla reserve)
    size_t totlen = 0;
    for (const auto& r : buffer_.rows) totlen += r.chars.size() + 1; // + '\n'

    std::string out;
    out.reserve(totlen);

    for (const auto& r : buffer_.rows) {
      out += r.chars;
      out += '\n';
    }
    return out;
  }

  void Editor::clamp_cursor() {
    if (cursor.y < 0) cursor.y = 0;
    if (cursor.y > (int)buffer_.rows.size()) cursor.y = (int)buffer_.rows.size();

    int rowlen = 0;
    if (cursor.y < (int)buffer_.rows.size()) {
      rowlen = (int)buffer_.rows[cursor.y].chars.size();
    }

    if (cursor.x < 0) cursor.x = 0;
    if (cursor.x > rowlen) cursor.x = rowlen;
  }

  void Editor::move_cursor(const char& key) {
    switch (key) {
    case 'h':
      cursor.x--;
      break;
    case 'd':
      cursor.x++;
      break;
    case 'w':
      cursor.y--;
      break;
    case 's':
      cursor.y++;
      break;
    }
  }

  void Editor::dispatch_action(const ::KeyAction& action) {
    std::visit([this](auto&& act) -> void {
      using T = std::decay_t<decltype(act)>;

      if constexpr (std::is_same_v<T, cursor::Action>) {
        handle_cursor_action(act);
      }
      else if constexpr (std::is_same_v<T, editor::Action>) {
        // handle_editor_action(act);
      }
      }, action);

  }

  void Editor::handle_cursor_action(const cursor::Action& action) {
    switch (action) {
    case cursor::Action::MoveLeft:
      if (cursor.x != 0) {
        cursor.x--;
      }
      else if (cursor.y > 0) {
        cursor.y--;
        cursor.x = (int)buffer_.rows[cursor.y].chars.size();
      }
      break;
    case cursor::Action::MoveRight: {
      if (cursor.y == (int)buffer_.rows.size()) break;

      int rowlen = (int)buffer_.rows[cursor.y].chars.size();

      if (cursor.x < rowlen) {
        cursor.x++;
      }
      else if (cursor.x == rowlen && cursor.y < (int)buffer_.rows.size() - 1) {
        cursor.y++;
        cursor.x = 0;
      }
      break;
    }
    case cursor::Action::MoveUp:
      if (cursor.y > 0) cursor.y--;
      break;
    case cursor::Action::MoveDown:
      if (cursor.y < (int)buffer_.rows.size()) cursor.y++;
      break;
    case cursor::Action::MovePageUp:
      cursor.y = (cursor.y > height_) ? (cursor.y - height_) : 0;
      break;
    case cursor::Action::MovePageDown:
      cursor.y = std::min(cursor.y + height_, (int)buffer_.rows.size());
      break;
    case cursor::Action::MoveHome:
      cursor.x = 0;
      break;
    case cursor::Action::MoveEnd:
      cursor.x = (cursor.y < (int)buffer_.rows.size())
        ? (int)buffer_.rows[cursor.y].chars.size()
        : 0;
      break;
    }
    clamp_cursor();
  }

  void Editor::clear_screen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
  }

  void Editor::die(const std::string error_code) {
    clear_screen();
    term.die(error_code);
  }
}