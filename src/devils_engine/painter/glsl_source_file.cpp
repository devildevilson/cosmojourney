#include "glsl_source_file.h"

#include "demiurg/module_interface.h"

namespace devils_engine {
namespace painter {
glsl_source_file::glsl_source_file() {
  set_flag(demiurg::resource_flags::warm_and_hot_same, true);
  set_flag(demiurg::resource_flags::binary, false);
}

// супер простой класс в котором мы просто ждем когда нас положат в какой нибудь шейдер
void glsl_source_file::load_cold(const utils::safe_handle_t &) {
  const auto str = module->load_text(path);
  file_memory = std::vector(str.cbegin(), str.cend());
  // нужно ли? по идее да, так мы можем контент взять как Си строку
  file_memory.push_back('\0');
  // std string_view не может быть Си строкой
  file_text = std::string_view(file_memory.data(), file_memory.size()-1);
}

void glsl_source_file::load_warm(const utils::safe_handle_t &) {}
void glsl_source_file::unload_hot(const utils::safe_handle_t &) {}
void glsl_source_file::unload_warm(const utils::safe_handle_t &) {}
}
}