#pragma once
#include <errno.h>
#include <termios.h>

#include <initializer_list>
#include <string>

#include "terminal/flags.hpp"
class Terminal {
public:
  Terminal();
  ~Terminal();

  void apply();
  void die(const std::string error_code);
  void restore();

  template <typename Flag>
  void enable(std::initializer_list<Flag> flags) {
    auto field = FlagCategory<Flag>::field;
    for (auto f : flags) {
      raw.*field |= static_cast<tcflag_t>(f);
    }
    apply();
  };
  template <typename Flag>
  void disable(std::initializer_list<Flag> flags) {
    auto field = FlagCategory<Flag>::field;
    for (auto f : flags) {
      raw.*field &= ~static_cast<tcflag_t>(f);
    }
    apply();
  };

private:
  termios orig, raw;
};
