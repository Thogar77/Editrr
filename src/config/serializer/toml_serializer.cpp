//
// Created by jakub on 6/12/26.
//

#include "editrr/config/serializer/toml_serializer.h"

#include <filesystem>

namespace serializer {
void TomlSerializer::serialize(const toml::table& data, const std::filesystem::path& file_path) {
  std::ofstream file{file_path, std::ios::trunc};
  if (!file.is_open()) {
    throw std::runtime_error{"Could not open file:" + file_path.string() + "for writing config"};
  }
  file << data;
}
}  // namespace serializer