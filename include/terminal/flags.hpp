#pragma once

#include <termios.h>
enum class LocalFlag : tcflag_t {
  Echo = ECHO,
  Canonical = ICANON,
  Signals = ISIG,
  Extended = IEXTEN,
};

enum class InputFlag : tcflag_t {
  XonXoff = IXON,
  CarriageReturn = ICRNL,
  ParityCheck = INPCK,
  Strip = ISTRIP,
  BreakCondition = BRKINT
};

enum class ControlFlag : tcflag_t { CharSize8 = CS8 };
enum class OutputFlag : tcflag_t { PostProcessing = OPOST };
template <typename T>
struct FlagCategory;

template <>
struct FlagCategory<LocalFlag> {
  static constexpr auto field = &termios::c_lflag;
};

template <>
struct FlagCategory<InputFlag> {
  static constexpr auto field = &termios::c_iflag;
};

template <>
struct FlagCategory<OutputFlag> {
  static constexpr auto field = &termios::c_oflag;
};

template <>
struct FlagCategory<ControlFlag> {
  static constexpr auto field = &termios::c_cflag;
};