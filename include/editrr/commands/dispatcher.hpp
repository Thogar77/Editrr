#pragma once
#include <memory>

#include "editrr/commands/command.hpp"
#include "editrr/config/app_config.hpp"
#include "editrr/input/key.hpp"

namespace editrr {

class Dispatcher {
 public:
  Dispatcher();
  std::unique_ptr<ICommand> map_key_to_command(const Key& k);

 private:
  config::AppConfig* cfg;
};

}  // namespace editrr
