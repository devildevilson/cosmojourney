#ifndef DEVILS_ENGINE_PAINTER_SYSTEM_H
#define DEVILS_ENGINE_PAINTER_SYSTEM_H

#include <cstddef>
#include <cstdint>
#include "vulkan_minimal.h"

// в системе нужно еще задать кеши, например пиплайн и скомпилированный шейдер кеш
// + добавить флаги чтобы не использовать кеши

// да еще в вулкане какая то тупая система чтобы найти нужный физикал девайс
// проще как то заранее отыскать с минимальным стейтом, потом передать (что?) в основную систему
// все пересоздав с нуля

namespace devils_engine {
namespace painter {
class system {
public:
  system();
  ~system();


private:
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkDevice device;
  VkQueue graphics_queue;
  VkQueue presentation_queue;
  VkPipelineCache cache;
};
}
}

#endif