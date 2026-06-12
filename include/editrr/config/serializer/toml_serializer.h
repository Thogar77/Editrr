//
// Created by jakub on 5/31/26.
//

#ifndef EDITRR_TOML_SERIALIZER_H
#define EDITRR_TOML_SERIALIZER_H
#include <filesystem>
#include <string>
#include <toml/toml.hpp>

#include "ISerializer.hpp"
#include "toml++/impl/table.hpp"

class TomlSerializer : ISerializer {
 public:
  TomlSerializer();
  virtual ~TomlSerializer();
  void serialize(const toml::table data, const std::filesystem::path& file_path) override;
};

#endif  // EDITRR_TOML_SERIALIZER_H
