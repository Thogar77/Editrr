//
// Created by jakub on 6/13/26.
//

#ifndef EDITRR_KEYBINDINGS_H
#define EDITRR_KEYBINDINGS_H
#include <magic_enum/magic_enum.hpp>
#include <string>
#include <toml++/toml.hpp>
#include <unordered_map>

#include "editrr/commands/command.hpp"
namespace config {
struct Keybindings {
  std::unordered_map<editrr::Command, char> commands{
      {editrr::Command::Quit, 'q'}, {editrr::Command::Save, 's'}, {editrr::Command::Find, 'f'}};
};
inline toml::table keybindings_to_toml(Keybindings keybindings) {
  toml::table keybinds;
  for (auto [key, value] : keybindings.commands) {
    const auto k = magic_enum::enum_name(key);
    keybinds.insert_or_assign(k, std::string(1, value));
  }
  toml::table result;
  result.insert_or_assign("keybindings", keybinds);
  return result;
}
}  // namespace config
#endif  // EDITRR_KEYBINDINGS_H
