#ifndef DEVILS_ENGINE_FILEIO_H
#define DEVILS_ENGINE_FILEIO_H

#include <cstdint>
#include <cstddef>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <span>
#include "core.h"

namespace devils_engine {
namespace file_io {
enum class type {
  binary,
  text,
  count
};

template <typename BYTE_TYPE = uint8_t>
std::vector<BYTE_TYPE> read(const std::string &path, const enum type type = type::binary);
std::string read(const std::string &path, const enum type type = type::text);
void write(const std::span<char> &bytes, const std::string &path, const enum type type = type::binary);
void write(const std::span<uint8_t> &bytes, const std::string &path, const enum type type = type::binary);
void write(const std::string_view &bytes, const std::string &path, const enum type type = type::text);
void write(const std::span<const char> &bytes, const std::string &path, const enum type type = type::binary);
void write(const std::span<const uint8_t> &bytes, const std::string &path, const enum type type = type::binary);
}
}

#endif