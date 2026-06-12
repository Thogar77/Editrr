#include "editrr/config/config.hpp"

#include <filesystem>
#include <iostream>

namespace config {

AppConfig* AppConfig::_instance = nullptr;
std::filesystem::path AppConfig::_config_path =
    std::filesystem::path(std::getenv("HOME")) / ".config" / "editrr";

AppConfig::AppConfig() {
  if (std::filesystem::exists(_config_path)) {
    create_default_config_file();
  }
}
AppConfig* AppConfig::instance() {
  if (!_instance) {
    static AppConfig instance;
    _instance = &instance;
  }
  return _instance;
}

void AppConfig::update() {}
void AppConfig::load() {}

bool AppConfig::set_config_path(const std::filesystem::path& path) {
  const auto home = std::filesystem::path(std::getenv("HOME"));
  if (std::filesystem::exists(home / path)) {
    _config_path = home / path;
    return true;
  };
  return false;
}
void AppConfig::create_default_config_file() {
  if (std::filesystem::exists(_config_path / "config.toml")) {
    return;
  }
}
}  // namespace config
