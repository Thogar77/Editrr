//
// Created by jakub on 6/13/26.
//

#ifndef EDITRR_KEYBINDINGS_H
#define EDITRR_KEYBINDINGS_H
#include <unordered_map>
#include <string>
#include <magic_enum/magic_enum.hpp>
#include "editrr/commands/command.hpp"
namespace editrr {
class Keybindings {
public:
  char get_command_key(const Command& command);
  bool load();
private:
  std::unordered_map<Command, char> Keybinding {
      {Command::Quit, 'q'},
      {Command::Save, 's'},
      {Command::Find, 'f'}
  } ;
};
}
#endif  // EDITRR_KEYBINDINGS_H
