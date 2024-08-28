// тут укажем все define так чтобы они не потерялись

#define VK_NO_PROTOTYPES
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.hpp>

#ifndef DEVILS_ENGINE_PAINTER_VULKAN_HEADER_H
#define DEVILS_ENGINE_PAINTER_VULKAN_HEADER_H

// так а тут может быть имплементация кое каких вещей
#include <cstdint>
#include <cstddef>
#include <string_view>
#include <reflect>
#include <bitset>
#include <tuple>
#include "utils/type_traits.h"
#include "utils/core.h"

namespace devils_engine {
namespace painter {

template <typename T>
void set_name(vk::Device device, T handle, const std::string &name) {
  vk::DebugUtilsObjectNameInfoEXT i(T::objectType, uint64_t(typename T::CType(obj)), name.c_str());
  device.setDebugUtilsObjectNameEXT(i);
}

template <typename T>
consteval void count_features(size_t &counter, const T &obj) {
  reflect::for_each([&](auto I) {
    using value_type = decltype(reflect::get<I>(obj));
    using mem_type = std::remove_cvref_t<value_type>;
    if constexpr (!std::is_same_v<mem_type, VkBool32>) return;
    counter += 1;
  }, obj);
}

template <typename T>
consteval bool find_feature(size_t &counter, const std::string_view &name, const T &obj) {
  bool found = false;

  reflect::for_each([&](auto I) {
    using value_type = decltype(reflect::get<I>(obj));
    using mem_type = std::remove_cvref_t<value_type>;
    if constexpr (!std::is_same_v<mem_type, VkBool32>) return;

    const std::string_view member_name = reflect::member_name<I>(obj);
    if (member_name == name) { found = true; return; }

    counter += 1;
  }, obj);

  return found;
}

consteval size_t count_vulkan_device_features() {
  size_t counter = 0;
  const auto obj10 = VkPhysicalDeviceFeatures{};
  const auto obj11 = VkPhysicalDeviceVulkan11Features{};
  const auto obj12 = VkPhysicalDeviceVulkan12Features{};
  const auto obj13 = VkPhysicalDeviceVulkan13Features{};

  //reflect::for_each([&](auto I) {
  //  using value_type = decltype(reflect::get<I>(obj10));
  //  using mem_type = std::remove_cvref_t<value_type>;
  //  //const auto member_name = reflect::member_name<I>(obj10);
  //  //const auto type_name = utils::type_name<mem_type>();
  //  //utils::println(type_name, member_name);
  //  //std::cout << type_name << " " << member_name << "\n";
  //  if constexpr (!std::is_same_v<mem_type, VkBool32>) return;
  //  counter += 1;
  //}, obj10);
  count_features(counter, obj10);
  count_features(counter, obj11);
  count_features(counter, obj12);
  count_features(counter, obj13);

  return counter;
}

consteval size_t get_feature_index(const std::string_view &name) {
  bool found = false;
  size_t counter = 0;
  const auto obj10 = VkPhysicalDeviceFeatures{};
  const auto obj11 = VkPhysicalDeviceVulkan11Features{};
  const auto obj12 = VkPhysicalDeviceVulkan12Features{};
  const auto obj13 = VkPhysicalDeviceVulkan13Features{};

  if (find_feature(counter, name, obj10)) return counter;
  if (find_feature(counter, name, obj11)) return counter;
  if (find_feature(counter, name, obj12)) return counter;
  if (find_feature(counter, name, obj13)) return counter;

  return SIZE_MAX;
}

constexpr size_t device_features_count = count_vulkan_device_features();
using vulkan_features_bitset = std::bitset<device_features_count>;
//using vulkan_features_bitset = std::bitset<256>;

// так теперь надо заполнить фичи в битовое поле
vulkan_features_bitset make_device_features_bitset(VkPhysicalDevice dev);


std::vector<vk::ExtensionProperties> required_device_extensions(vk::PhysicalDevice device, const std::vector<const char*> &layers, const std::vector<const char*> &extensions);
std::vector<vk::LayerProperties> required_validation_layers(const std::vector<const char*> &layers);

std::tuple<vk::Image, vma::Allocation> create_image(
  vma::Allocator allocator, 
  const vk::ImageCreateInfo &info,
  const vma::MemoryUsage &mem_usage,
  void** pData = nullptr,
  const std::string &name = ""
);

std::tuple<vk::AccessFlags, vk::AccessFlags, vk::PipelineStageFlags, vk::PipelineStageFlags> make_barrier_data(const vk::ImageLayout &old, const vk::ImageLayout &New);

std::tuple<vk::ImageMemoryBarrier, vk::PipelineStageFlags, vk::PipelineStageFlags> make_image_memory_barrier(
  vk::Image image, const vk::ImageLayout &old_layout, const vk::ImageLayout &new_layout, const vk::ImageSubresourceRange &range
);

void change_image_layout(
  vk::Device device, 
  vk::Image image, 
  vk::CommandPool transfer_pool, 
  vk::Queue transfer_queue, 
  vk::Fence fence, 
  const vk::ImageLayout &old_layout, 
  const vk::ImageLayout &new_layout, 
  const vk::ImageSubresourceRange &range
);

}
}

#endif