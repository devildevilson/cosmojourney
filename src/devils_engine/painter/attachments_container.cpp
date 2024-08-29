#include "attachments_container.h"

#include "vulkan_header.h"

#include "auxiliary.h"

namespace devils_engine {
namespace painter {
attachments_container::attachments_container(VkInstance instance, VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface, std::vector<uint32_t> formats) :
  instance(instance), device(device), physical_device(physical_device), surface(surface), formats(std::move(formats)), width(0), height(0), allocator(VK_NULL_HANDLE), swapchain(VK_NULL_HANDLE), current_image(0)
{
  // какая буферизация? предположим двойная
  attachments.resize(2);

  // allocator может быть тоже переделывать по размерам изображений? вообще возможно имеет смысл
}

attachments_container::~attachments_container() noexcept {
  clear();
  destroy_swapchain();
  //vma::Allocator(allocator).destroy();
}

uint32_t attachments_container::max_frames() const { return attachments.size(); }
size_t attachments_container::get_views(const uint32_t frame_index, VkImageView* views, const size_t max_size) const {
  if (frame_index >= max_frames()) return 0;

  size_t i = 0;
  for (; i < std::min(max_size, attachments[frame_index].size()); ++i) {
    const auto &att = attachments[frame_index][i];
    views[i] = att.view;
  }

  return i;
}

void attachments_container::recreate(const uint32_t width, const uint32_t height) {
  clear();

  recreate_swapchain(width, height);
  recreate_images(width, height);
  
  this->width = width;
  this->height = height;
}

uint32_t attachments_container::acquire_next_image(VkSemaphore semaphore, uint32_t &current_image) {
  vk::Device dev(device);

  const auto [ res, index ] = dev.acquireNextImageKHR(swapchain, SIZE_MAX, semaphore, VK_NULL_HANDLE);
  this->current_image = index;
  current_image = index;
  return uint32_t(res);
}

void attachments_container::clear() {
  vk::Device dev(device);
  vma::Allocator a(allocator);

  for (auto & arr : attachments) {
    for (auto & att : arr) {
      if (att.alloc != VK_NULL_HANDLE) { a.destroyImage(att.image, att.alloc); }
      dev.destroy(att.view);
    }
    arr.clear();
  }

  a.destroy();
}

void attachments_container::destroy_swapchain() {
  vk::Device dev(device);
  if (swapchain != VK_NULL_HANDLE) {
    dev.destroy(swapchain);
    swapchain = VK_NULL_HANDLE;
  }
}

void attachments_container::recreate_swapchain(const uint32_t width, const uint32_t height) {
  vk::Device dev(device);
  vk::PhysicalDevice pd(physical_device);

  const auto caps = pd.getSurfaceCapabilitiesKHR(surface);
  const auto surface_formats = pd.getSurfaceFormatsKHR(surface);
  const auto modes = pd.getSurfacePresentModesKHR(surface);

  const auto mode = choose_swapchain_present_mode(modes);
  const auto surface_format = choose_swapchain_surface_format(surface_formats);
  const auto ext = choose_swapchain_extent(width, height, caps);
  if (ext.width != width || ext.height != height) {
    utils::error("Wut? {} == {} == {} && {} == {} == {}", width, caps.currentExtent.width, ext.width, height, caps.currentExtent.height, ext.height);
  }

  const uint32_t max_count = caps.maxImageCount > 0 ? caps.maxImageCount : 3;
  const auto image_count = std::clamp(uint32_t(attachments.size()), caps.minImageCount, max_count);
  if (image_count != attachments.size()) utils::error("This vulkan device does not supports double buffering");
  
  const auto usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
  const vk::SwapchainCreateInfoKHR inf(
    {}, vk::SurfaceKHR(surface), image_count, surface_format.format, surface_format.colorSpace, 
    caps.currentExtent, 1, usage, vk::SharingMode::eExclusive, 0, nullptr, 
    caps.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, 
    mode, VK_TRUE, vk::SwapchainKHR(swapchain)
  );

  auto new_swapchain = dev.createSwapchainKHR(inf);

  destroy_swapchain();

  swapchain = new_swapchain;
  set_name(device, swapchain, "window_swapchain");

  const auto images = dev.getSwapchainImagesKHR(swapchain);
  if (images.size() != attachments.size()) {
    utils::error("This vulkan device does not supports double buffering");
  }

  for (size_t i = 0; i < attachments.size(); ++i) {
    auto &arr = attachments[i];
    arr.resize(formats.size());
    arr[0].image = images[i];
    arr[0].format = uint32_t(surface_format.format);
    set_name(device, images[i], "swapchain image " + std::to_string(i));
  }
}

void attachments_container::recreate_images(const uint32_t width, const uint32_t height) {
  vk::Device dev(device);
  const size_t heap_block_size = width * height * 4 * formats.size();
  const auto f = make_functions();
  vma::AllocatorCreateInfo inf({}, physical_device, device, heap_block_size, nullptr, nullptr, nullptr, &f, instance, VK_API_VERSION_1_0);
  allocator = vma::createAllocator(inf);

  for (auto & arr : attachments) {
    for (size_t i = 0; i < arr.size(); ++i) {
      auto &att = arr[i];
      const auto format = formats[i];
      att.format = format;
      const auto vk_format = vk::Format(format);
      const auto format_name = vk::to_string(vk_format);

      if (att.image == VK_NULL_HANDLE) {
        const auto att_usage = main_attachment_usage_from_format(vk_format);
        const auto usage = att_usage | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eTransferSrc;
        auto [img, mem] = create_image(allocator, texture2D({ width, height }, usage, vk_format), vma::MemoryUsage::eGpuOnly, nullptr, format_name + " attachment");
        att.image = img;
        att.alloc = mem;
      }

      auto inf = make_view_info(att.image, vk_format);
      // здесь можно добавить свизлинг, чтобы цвета были адекватными ргб
      // челик на гитхабе грит что компонент свиззл может быть недоступен где то, как проверить?
      const auto swizzle = to_rgba(vk_format);
      //inf.components = swizzle;
      att.view = dev.createImageView(inf);
    }
  }
}


}
}