#pragma once
#include <memory>

#include "editrr/commands/command.hpp"
#include "editrr/input/key.hpp"

namespace editrr {

class Dispatcher {
 public:
  std::unique_ptr<ICommand> map_key_to_command(const Key& k);
};

}  // namespace editrr
