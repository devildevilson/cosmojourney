#ifndef DEVILS_ENGINE_PAINTER_COMMON_H
#define DEVILS_ENGINE_PAINTER_COMMON_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace devils_engine {
namespace painter {
struct cached_system_data {
  std::string device_name;
  std::string device_type;
  uint32_t device_id;
  uint32_t vendor_id;
  uint32_t graphics_queue;
  uint32_t compute_queue;
  uint32_t present_queue;
  std::string desirable_present_mode;
  std::string fallback_present_mode;
  size_t memory_capacity;
};
}
}

#endif