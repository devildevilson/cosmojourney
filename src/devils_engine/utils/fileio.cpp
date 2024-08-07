#include "fileio.h"

namespace devils_engine {
namespace file_io {
template <>
std::vector<char> read(const std::string &path, const enum type type) {
  const auto flags = type == type::binary ? std::ios::in | std::ios::binary : std::ios::in;
  std::ifstream file(path, flags);
  if (!file) utils::error("Could not load file {}", path);
  return std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

template <>
std::vector<uint8_t> read(const std::string &path, const enum type type) {
  const auto flags = type == type::binary ? std::ios::in | std::ifstream::ate | std::ios::binary : std::ios::in | std::ifstream::ate;
  std::ifstream file(path, flags);
  if (!file) utils::error("Could not load file {}", path);
  const size_t file_size = file.tellg();
  file.seekg(0);
  std::vector<uint8_t> ret(file_size, 0);
  file.read(reinterpret_cast<char *>(ret.data()), ret.size());
  return ret;
}

std::string read(const std::string &path, const enum type type) {
  const auto flags = type == type::binary ? std::ios::in | std::ios::binary : std::ios::in;
  std::ifstream file(path, flags);
  if (!file) utils::error("Could not load file {}", path);
  return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void write(const std::span<char> &bytes, const std::string &path, const enum type type) {
  const auto flags = type == type::binary ? std::ios::out | std::ios::binary | std::ios::trunc : std::ios::out | std::ios::trunc;
  std::ofstream file(path, flags);
  file.write(bytes.data(), bytes.size());
}

void write(const std::span<uint8_t> &bytes, const std::string &path, const enum type type) {
  const auto flags = type == type::binary ? std::ios::out | std::ios::binary | std::ios::trunc : std::ios::out | std::ios::trunc;
  std::ofstream file(path, flags);
  file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

void write(const std::string_view &bytes, const std::string &path, const enum type type) {
  const auto flags = type == type::binary ? std::ios::out | std::ios::binary | std::ios::trunc : std::ios::out | std::ios::trunc;
  std::ofstream file(path, flags);
  file.write(bytes.data(), bytes.size());
}

void write(const std::span<const char> &bytes, const std::string &path, const enum type type) {
  const auto flags = type == type::binary ? std::ios::out | std::ios::binary | std::ios::trunc : std::ios::out | std::ios::trunc;
  std::ofstream file(path, flags);
  file.write(bytes.data(), bytes.size());
}

void write(const std::span<const uint8_t> &bytes, const std::string &path, const enum type type) {
  const auto flags = type == type::binary ? std::ios::out | std::ios::binary | std::ios::trunc : std::ios::out | std::ios::trunc;
  std::ofstream file(path, flags);
  file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}
}
}