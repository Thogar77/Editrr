#pragma once
#include <string>
#include <toml++/toml.hpp>

#include "editrr/config/types/types.hpp"
namespace parser {
class TomlParser : public IParser {
 public:
  toml::table parse(const std::filesystem::path& path);
};
}  // namespace parser