#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vma/vk_mem_alloc.h>

#include "vulkan_header.h"

#include "auxiliary.h"

#include <parallel_hashmap/phmap.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace devils_engine {
namespace painter {
template <typename T>
void get_features(size_t &counter, vulkan_features_bitset &bitset, const T &obj) {
  reflect::for_each([&](auto I) {
    using value_type = decltype(reflect::get<I>(obj));
    using mem_type = std::remove_cvref_t<value_type>;
    if constexpr (!std::is_same_v<mem_type, VkBool32>) return;

    const bool val = reflect::get<I>(obj);
    bitset.set(counter, val);
    
    counter += 1;
  }, obj);
}

vulkan_features_bitset make_device_features_bitset(VkPhysicalDevice dev) {
  vulkan_features_bitset ret;
  vk::PhysicalDevice d(dev);

  const auto f = d.getFeatures2();
  const auto &fs10 = VkPhysicalDeviceFeatures(f.features);

  size_t counter = 0;
  get_features(counter, ret, fs10);

  for (auto ptr = reinterpret_cast<const VkBaseInStructure*>(f.pNext); ptr != nullptr; ptr = ptr->pNext) {
    if (ptr->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES) {
      auto fs11 = *reinterpret_cast<const VkPhysicalDeviceVulkan11Features*>(ptr);
      get_features(counter, ret, fs11);
      continue;
    }

    if (ptr->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES) {
      auto fs12 = *reinterpret_cast<const VkPhysicalDeviceVulkan12Features*>(ptr);
      get_features(counter, ret, fs12);
      continue;
    }

    if (ptr->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES) {
      auto fs13 = *reinterpret_cast<const VkPhysicalDeviceVulkan13Features*>(ptr);
      get_features(counter, ret, fs13);
      continue;
    }
  }

  return ret;
}

std::vector<vk::ExtensionProperties> required_device_extensions(vk::PhysicalDevice device, const std::vector<const char*> &layers, const std::vector<const char*> &extensions) {
  std::vector<vk::ExtensionProperties> finalExtensions;

  const auto ext = device.enumerateDeviceExtensionProperties(nullptr);

  phmap::flat_hash_set<std::string> intersection(extensions.begin(), extensions.end());

  for (const auto &extension : ext) {
    if (intersection.find(extension.extensionName) != intersection.end()) finalExtensions.push_back(extension);
  }

  for (auto layer : layers) {
    const auto ext = device.enumerateDeviceExtensionProperties(std::string(layer));

    for (const auto &extension : ext) {
      if (intersection.find(extension.extensionName) != intersection.end()) finalExtensions.push_back(extension);
    }
  }

  return finalExtensions;
}

std::vector<vk::LayerProperties> required_validation_layers(const std::vector<const char*> &layers) {
  const auto &availableLayers = vk::enumerateInstanceLayerProperties();

  phmap::flat_hash_set<std::string> intersection(layers.begin(), layers.end());
  std::vector<vk::LayerProperties> finalLayers;

  for (const auto &layer : availableLayers) {
    auto itr = intersection.find(std::string(layer.layerName.data()));
    if (itr != intersection.end()) finalLayers.push_back(layer);
  }

  return finalLayers;
}

std::tuple<vk::Image, vma::Allocation> create_image(
  vma::Allocator allocator, 
  const vk::ImageCreateInfo &info,
  const vma::MemoryUsage &mem_usage,
  void** pData,
  const std::string &name
) {
  const bool need_memory_map = mem_usage == vma::MemoryUsage::eCpuOnly || mem_usage == vma::MemoryUsage::eCpuCopy || mem_usage == vma::MemoryUsage::eCpuToGpu;
  const auto fl = need_memory_map ? vma::AllocationCreateFlagBits::eMapped : vma::AllocationCreateFlags();
  const vma::AllocationCreateInfo alloc_info(fl, mem_usage);
  std::pair<vk::Image, vma::Allocation> p;
  if (pData == nullptr) {
    p = allocator.createImage(info, alloc_info);
  } else {
    vma::AllocationInfo i;
    p = allocator.createImage(info, alloc_info, &i);
    *pData = i.pMappedData;
  }
  auto dev = allocator_device(allocator);
  set_name(dev, p.first, name);
  return std::make_tuple(p.first, p.second);
}

std::tuple<vk::AccessFlags, vk::AccessFlags, vk::PipelineStageFlags, vk::PipelineStageFlags> 
  make_barrier_data(const vk::ImageLayout &old, const vk::ImageLayout &New) 
{
  vk::AccessFlags srcFlags, dstFlags; 
  vk::PipelineStageFlags srcStage, dstStage;
      
  switch (old) {
    case vk::ImageLayout::eUndefined:
      srcFlags = vk::AccessFlags(0);
      srcStage = vk::PipelineStageFlagBits::eTopOfPipe;

      break;
    case vk::ImageLayout::ePreinitialized:
      srcFlags = vk::AccessFlagBits::eHostWrite;
      srcStage = vk::PipelineStageFlagBits::eHost;

      break;
    case vk::ImageLayout::eGeneral:
      srcFlags = vk::AccessFlagBits::eShaderWrite;
      srcStage = vk::PipelineStageFlagBits::eComputeShader;

      break;
    case vk::ImageLayout::eColorAttachmentOptimal:
      srcFlags = vk::AccessFlagBits::eColorAttachmentWrite;
      srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

      break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
      srcFlags = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      srcStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;

      break;
    case vk::ImageLayout::eTransferSrcOptimal:
      srcFlags = vk::AccessFlagBits::eTransferRead;
      srcStage = vk::PipelineStageFlagBits::eTransfer;

      break;
    case vk::ImageLayout::eTransferDstOptimal:
      srcFlags = vk::AccessFlagBits::eTransferWrite;
      srcStage = vk::PipelineStageFlagBits::eTransfer;

      break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
      srcFlags = vk::AccessFlagBits::eShaderRead;
      srcStage = vk::PipelineStageFlagBits::eFragmentShader;

      break;
    case vk::ImageLayout::ePresentSrcKHR:
      srcFlags = vk::AccessFlagBits::eMemoryRead;
      srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

      break;
    default: utils::error("The layout '{}' is not supported yet", vk::to_string(old)); break;
  }

  switch (New) {
    case vk::ImageLayout::eColorAttachmentOptimal:
      dstFlags = vk::AccessFlagBits::eColorAttachmentWrite;
      dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

      break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
      dstFlags = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;

      break;
    case vk::ImageLayout::eTransferSrcOptimal:
      dstFlags = vk::AccessFlagBits::eTransferRead;
      dstStage = vk::PipelineStageFlagBits::eTransfer;

      break;
    case vk::ImageLayout::eTransferDstOptimal:
      dstFlags = vk::AccessFlagBits::eTransferWrite;
      dstStage = vk::PipelineStageFlagBits::eTransfer;

      break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
      if (srcFlags == vk::AccessFlags(0)) {
        srcFlags = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
        srcStage = vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eHost;
      }

      dstFlags = vk::AccessFlagBits::eShaderRead;
      dstStage = vk::PipelineStageFlagBits::eFragmentShader;

      break;
    case vk::ImageLayout::ePresentSrcKHR:
      dstFlags = vk::AccessFlagBits::eMemoryRead;
      dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

      break;
    case vk::ImageLayout::eGeneral:
      dstFlags = vk::AccessFlagBits::eShaderWrite;
      dstStage = vk::PipelineStageFlagBits::eComputeShader;

      break;
    default: utils::error("The layout '{}' is not supported yet", vk::to_string(New)); break;
  }
      
  return std::make_tuple(srcFlags, dstFlags, srcStage, dstStage);
}

std::tuple<vk::ImageMemoryBarrier, vk::PipelineStageFlags, vk::PipelineStageFlags> make_image_memory_barrier(
  vk::Image image, const vk::ImageLayout &old_layout, const vk::ImageLayout &new_layout, const vk::ImageSubresourceRange &range
) {
  vk::ImageMemoryBarrier b({}, {}, old_layout, new_layout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, range);
  const auto [srcFlags, dstFlags, srcStage, dstStage] = make_barrier_data(old_layout, new_layout);
  b.srcAccessMask = srcFlags;
  b.dstAccessMask = dstFlags;
      
  return std::make_tuple(b, srcStage, dstStage);
}

void change_image_layout(
  vk::Device device, 
  vk::Image image, 
  vk::CommandPool transfer_pool, 
  vk::Queue transfer_queue, 
  vk::Fence fence, 
  const vk::ImageLayout &old_layout, 
  const vk::ImageLayout &new_layout, 
  const vk::ImageSubresourceRange &range
) {
  do_command(device, transfer_pool, transfer_queue, fence, [&] (VkCommandBuffer t) {
    auto task = vk::CommandBuffer(t);
    const auto [b_info, srcStage, dstStage] = make_image_memory_barrier(image, old_layout, new_layout, range);
    const vk::CommandBufferBeginInfo binfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    task.begin(binfo);
    task.pipelineBarrier(srcStage, dstStage, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, b_info);
    task.end();
  });
}

}
}