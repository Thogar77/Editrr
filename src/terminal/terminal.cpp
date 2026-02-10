#include "terminal/terminal.hpp"

#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

Terminal::Terminal() {
  if (tcgetattr(STDIN_FILENO, &orig) == -1) {
    die("Failed to get terminal attributes");
  }
  raw = orig;
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
}

void Terminal::die(const std::string error_code) {
  restore();
  throw std::runtime_error(error_code.c_str());
}

Terminal::~Terminal() {
  restore();
}

void Terminal::restore() {
  std::cout << "Restoring terminal settings...\n";
  enable({LocalFlag::Echo, LocalFlag::Canonical, LocalFlag::Signals, LocalFlag::Extended});
  enable({InputFlag::XonXoff, InputFlag::CarriageReturn, InputFlag::BreakCondition,
          InputFlag::Strip, InputFlag::ParityCheck});
  enable({OutputFlag::PostProcessing});
  disable({ControlFlag::CharSize8});
}

void Terminal::apply() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("Died at applying config!");
  }
}
