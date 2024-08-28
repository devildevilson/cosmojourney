#ifndef DEVILS_ENGINE_PAINTER_AUXILIARY_H
#define DEVILS_ENGINE_PAINTER_AUXILIARY_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string_view>
#include <functional>
#include "common.h"
#include "vulkan_minimal.h"

typedef struct GLFWwindow GLFWwindow;

namespace devils_engine {
namespace painter {
const std::vector<const char*> default_validation_layers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> default_device_extensions = { "VK_KHR_swapchain" };

bool check_validation_layer_support(const std::vector<const char *> &layers);
std::vector<const char *> get_required_extensions();

void load_dispatcher1();
void load_dispatcher2(VkInstance inst);
void load_dispatcher3(VkDevice device);

VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance inst);
void destroy_debug_messenger(VkInstance inst, VkDebugUtilsMessengerEXT handle);

bool physical_device_presentation_support(VkInstance inst, VkPhysicalDevice dev, const uint32_t queue_family_index);
VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow* window);
void destroy_surface(VkInstance instance, VkSurfaceKHR surface);

VkPhysicalDevice find_device_process(VkInstance instance, cached_system_data* cached_data = nullptr);

bool do_command(VkDevice device, VkCommandPool pool, VkQueue queue, VkFence fence, std::function<void(VkCommandBuffer)> action);
void copy_buffer(VkDevice device, VkCommandPool pool, VkQueue queue, VkFence fence, VkBuffer src, VkBuffer dst, size_t srcoffset = 0, size_t dstoffset = 0, size_t size = VK_WHOLE_SIZE);
//void copy_image(VkDevice device, VkCommandPool pool, VkQueue queue, VkFence fence, VkImage dst, VkImage src, );

VkDevice allocator_device(VmaAllocator allocator);
VkInstance allocator_instance(VmaAllocator allocator);
}
}

#endif