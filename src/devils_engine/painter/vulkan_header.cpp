#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vma/vk_mem_alloc.h>

#include "vulkan_header.h"

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

}
}