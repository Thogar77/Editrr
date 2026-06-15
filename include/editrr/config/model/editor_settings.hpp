//
// Created by jakub on 6/13/26.
//

#ifndef EDITRR_EDITOR_SETTINGS_HPP
#define EDITRR_EDITOR_SETTINGS_HPP
#include <cstdint>
#include <filesystem>

#include "editrr/config/model/fonts.hpp"
namespace config {
struct EditorSettings {
  uint8_t tab_width{4};
  bool line_numbers{true};
  font font_{font::DEFAULT};
  std::filesystem::path settings_path;
};
}  // namespace config
#endif  // EDITRR_EDITOR_SETTINGS_HPP
