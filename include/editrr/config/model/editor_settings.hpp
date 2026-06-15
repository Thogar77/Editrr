//
// Created by jakub on 6/13/26.
//

#ifndef EDITRR_EDITOR_SETTINGS_HPP
#define EDITRR_EDITOR_SETTINGS_HPP
#include <cstdint>
#include "editrr/config/model/fonts.hpp"
namespace editrr {
class EditorSettings {
  public:
  EditorSettings();
private:
  uint8_t tab_width{4};
  bool line_numbers{true};
  font font{font::DEFAULT};
};
}
#endif  // EDITRR_EDITOR_SETTINGS_HPP
