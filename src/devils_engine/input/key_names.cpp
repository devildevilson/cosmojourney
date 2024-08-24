#include "key_names.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else

#endif

namespace devils_engine {
namespace input {
size_t get_key_name(const int32_t scancode, char* buffer, const size_t max_size) {
#ifdef _WIN32
  const size_t wchar_buf_size = 256;
  wchar_t buf[wchar_buf_size]{};
  const size_t count = GetKeyNameTextW(LONG(scancode) << 16, buf, wchar_buf_size);
  const auto err = _wcslwr_s(buf, wchar_buf_size);
  return wcstombs(buffer, buf, max_size);
#else
  
#endif
}

size_t get_key_name(const int32_t scancode, key_name_buffer& buffer) {
  return get_key_name(scancode, buffer, 256);
}

std::string get_key_name(const int32_t scancode) {
  char buf[512] {};
  const size_t count = get_key_name(scancode, buf, 512);
  return std::string(buf, count);
}
}
}