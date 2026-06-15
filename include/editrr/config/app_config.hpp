#pragma once
#include <array>
#include <memory>
#include <string>

#include "editrr/config/types/types.hpp"
#include "editrr/config/serializer/toml_serializer.h"
#include "editrr/config/model/keybindings.hpp"
#include "editrr/config/model/editor_settings.hpp"
namespace config {

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
  KeyBindings _keybindings;
  editrr::EditorSettings _editor_settings;
};
}  // namespace config