#pragma once
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "editor/key_mappings.hpp"
#include "terminal/terminal.hpp"
struct Cursor {
  int x;
  int y;
};

class Editor {
 public:
  Editor();
  void run();
  void enable_raw_mode();
  void restore();

 private:
  Cursor cursor;
  Terminal term;
  int width_;
  int height_;

  void initialize();
  char read_key();
  void process_keypress();
  void refresh_screen();
  void clear_screen();
  void draw_rows(std::string& buffer);
  int get_window_size(int& rows, int& cols);
  int get_cursor_position(int& rows, int& cols);
  void die(const std::string error_code);
};
