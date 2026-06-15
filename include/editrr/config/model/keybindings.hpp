//
// Created by jakub on 6/13/26.
//

#ifndef EDITRR_KEYBINDINGS_H
#define EDITRR_KEYBINDINGS_H
#include <magic_enum/magic_enum.hpp>
#include <string>
#include <unordered_map>

#include "editrr/commands/command.hpp"
#include "toml++/impl/table.hpp"
namespace config {
struct Keybindings {
  std::unordered_map<editrr::Command, char> commands{
      {editrr::Command::Quit, 'q'}, {editrr::Command::Save, 's'}, {editrr::Command::Find, 'f'}};
};
toml::table keybindings_to_toml(Keybindings keybindings);
}  // namespace config
#endif  // EDITRR_KEYBINDINGS_H
