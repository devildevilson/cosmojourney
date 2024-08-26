#ifndef DEVILS_ENGINE_PAINTER_AUXILIARY_H
#define DEVILS_ENGINE_PAINTER_AUXILIARY_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string_view>
#include "vulkan_minimal.h"

typedef struct GLFWwindow GLFWwindow;

namespace devils_engine {
namespace painter {
const std::vector<const char*> default_validation_layers = { "VK_LAYER_KHRONOS_validation" };

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
}
}

#endif