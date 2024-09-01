#ifndef DEVILS_ENGINE_PAINTER_SWAPCHAIN_RESOURCES_H
#define DEVILS_ENGINE_PAINTER_SWAPCHAIN_RESOURCES_H

#include <cstddef>
#include <cstdint>
#include "primitives.h"

namespace devils_engine {
namespace painter {

// вообще у свопчейна может быть прилично настроек
class simple_swapchain : public frame_acquisitor, public recreate_target {
public:
  simple_swapchain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface, const uint32_t buffering_target);
  ~simple_swapchain() noexcept;

  uint32_t acquire_next_image(size_t timeout, VkSemaphore semaphore, VkFence fence) override;
  VkImage frame_storage(const uint32_t index) const override;
  uint32_t frame_format(const uint32_t index) const override;
  void recreate(const uint32_t width, const uint32_t height) override;
  void destroy_swapchain();
protected:
  VkDevice device;
  VkPhysicalDevice physical_device;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
  uint32_t format;
  std::vector<VkImage> images;
};

}
}

#endif