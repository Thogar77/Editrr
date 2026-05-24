#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace editrr {

constexpr int TABSTOP = 8;

enum class Highlight : uint8_t { Normal = 0, Number, String, Comment, Keyword1, Keyword2 };

struct Row {
  size_t buf_offset{0};
  size_t length{0};

  bool dirty{true};
  std::string render;
  std::vector<Highlight> hl;
  bool hl_open_comment{false};

  void set_dirty(bool v) { dirty = v; }
};

}  // namespace editrr