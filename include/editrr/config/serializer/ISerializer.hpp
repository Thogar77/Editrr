#pragma once
#include <filesystem>
#include <toml++/toml.hpp>

class ISerializer {
 public:
  virtual ~ISerializer() = default;
  virtual void serialize(const toml::table& data, const std::filesystem::path& file_path) = 0;
};