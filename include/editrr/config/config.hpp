#pragma once
#include <array>
#include <memory>
#include <string>

#include "editrr/config/types/types.hpp"
namespace config {
enum class Keybinding : char { QUIT = 'q', SAVE = 's', FIND = 'f' };

inline std::array<std::string, 1> configs{"keybindings"};
class AppConfig : public IConfig {
 public:
  void load() override;
  void update() override;

  static AppConfig* instance();

  static bool set_config_path(const std::filesystem::path& path);
  static std::filesystem::path get_config_path() {return _config_path;};
 private:
  AppConfig();
  void create_default_config_file();
  static std::filesystem::path _config_path;
  static AppConfig* _instance;
};
}  // namespace config