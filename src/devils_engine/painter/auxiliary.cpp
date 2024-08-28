#include "auxiliary.h"

#include "vulkan_header.h"

#include "GLFW/glfw3.h"
#include "input/core.h"

#include "system_info.h"

#include <ranges>
namespace rv = std::ranges::views;

namespace devils_engine {
namespace painter {

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* pUserData
) {
  utils::println("validation layer:", pCallbackData->pMessage);
  return VK_FALSE;
}

bool check_validation_layer_support(const std::vector<const char*>& layers) {
  const auto layers_pred = [&](const auto &a) {
    const auto pred = [&](const auto &b) { return strcmp(a.layerName, b) == 0; };
    return std::ranges::count_if(layers, pred) != 0;
  };

  const auto layers_props = vk::enumerateInstanceLayerProperties();
  return std::ranges::count_if(layers_props, layers_pred) == layers.size();
}

std::vector<const char*> get_required_extensions() {
  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

  if (enable_validation_layers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

void load_dispatcher1() { 
  if (!glfwVulkanSupported()) utils::error("Vulkan is not supported by this system");
  VULKAN_HPP_DEFAULT_DISPATCHER.init();  
  glfwInitVulkanLoader(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr);
}

void load_dispatcher2(VkInstance inst) { VULKAN_HPP_DEFAULT_DISPATCHER.init(vk::Instance(inst)); }
void load_dispatcher3(VkDevice device) { VULKAN_HPP_DEFAULT_DISPATCHER.init(vk::Device(device)); }

VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance inst) {
  const auto severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;

  const auto type = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding;

  vk::DebugUtilsMessengerCreateInfoEXT info({}, severity, type, &debug_callback, nullptr);

  vk::Instance instance(inst);
  VkDebugUtilsMessengerEXT handle = instance.createDebugUtilsMessengerEXT(info);
  return handle;
}

void destroy_debug_messenger(VkInstance inst, VkDebugUtilsMessengerEXT handle) {
  vk::Instance instance(inst);
  instance.destroyDebugUtilsMessengerEXT(handle);
}

bool physical_device_presentation_support(VkInstance inst, VkPhysicalDevice dev, const uint32_t queue_family_index) {
  return glfwGetPhysicalDevicePresentationSupport(inst, dev, queue_family_index) == GLFW_TRUE;
}

VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow* window) {
  VkSurfaceKHR surf = VK_NULL_HANDLE;
  const auto res = glfwCreateWindowSurface(instance, window, nullptr, &surf);
  if (res != VK_SUCCESS) utils::error("Could not create surface from window '{}', result: {}", input::window_title(window), vk::to_string(vk::Result(res)));
  return surf;
}

void destroy_surface(VkInstance instance, VkSurfaceKHR surface) {
  vk::Instance(instance).destroySurfaceKHR(surface);
}

VkPhysicalDevice find_device_process(VkInstance i, cached_system_data* cached_data) {
  painter::system_info inf(i);
  auto w = input::create_window(640, 480, "test", nullptr, nullptr);
  auto surf = painter::create_surface(inf.instance, w);
  inf.check_devices_surface_capability(surf);
  const auto dev = inf.choose_physical_device();

  inf.dump_cache_to_disk(dev, cached_data);

  painter::destroy_surface(inf.instance, surf);
  input::destroy(w);

  // инстанса уже тут не будет, мы можем чуть переделать структуру system_info
  // передав туда инстанс, вот теперь девайс должен быть валидным указателем
  return dev;
}

bool do_command(VkDevice device, VkCommandPool pool, VkQueue queue, VkFence fence, std::function<void(VkCommandBuffer)> action) {
  vk::Device d(device);
  vk::CommandBufferAllocateInfo ai(pool, vk::CommandBufferLevel::ePrimary, 1);
  const auto buffer = std::move(d.allocateCommandBuffersUnique(ai)[0]);

  action(buffer.get());

  const vk::SubmitInfo si(nullptr, nullptr, buffer.get(), nullptr);
  vk::Queue(queue).submit(si, fence);

  const auto res = d.waitForFences(vk::Fence(fence), VK_TRUE, SIZE_MAX);
  d.resetFences(vk::Fence(fence));

  return res == vk::Result::eSuccess;
}

void copy_buffer(VkDevice device, VkCommandPool pool, VkQueue queue, VkFence fence, VkBuffer src, VkBuffer dst, size_t srcoffset, size_t dstoffset, size_t size) {
  do_command(device, pool, queue, fence, [&] (VkCommandBuffer buffer) {
    auto b = vk::CommandBuffer(buffer);
    const vk::CommandBufferBeginInfo binfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    b.begin(binfo);
    b.copyBuffer(src, dst, vk::BufferCopy(srcoffset, dstoffset, size));
    b.end();
  });
}

VkDevice allocator_device(VmaAllocator allocator) { return (*allocator).m_hDevice; }
VkInstance allocator_instance(VmaAllocator allocator) { return (*allocator).m_hInstance; }

}
}