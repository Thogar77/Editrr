#include "editrr/config/parser/parser.hpp"

namespace parser {
toml::table TomlParser::parse(const std::filesystem::path& path) { return toml::parse(std::string(path)); }
}  // namespace parser