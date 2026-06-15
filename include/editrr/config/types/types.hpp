#pragma once
#include <filesystem>
#include <string>
class IParser {
 public:
  template <class T>
  T parse(std::filesystem::path path);
};

class IConfig {
 public:
  virtual void load() = 0;
  virtual void update() = 0;
};