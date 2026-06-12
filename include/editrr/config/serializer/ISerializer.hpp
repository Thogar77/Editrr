#include <filesystem>
#include <toml++/toml.hpp>
class ISerializer {
 public:
  void serialize(const toml::table data, const std::filesystem::path& file_path);
};