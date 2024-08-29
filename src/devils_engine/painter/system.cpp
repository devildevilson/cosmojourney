#include "system.h"

#include "vulkan_header.h"
#include "common.h"
#include "auxiliary.h"
#include "utils/named_serializer.h"
#include "utils/fileio.h"
#include "utils/core.h"
#include "utils/compression.h"

#include "makers.h"

namespace devils_engine {
namespace painter {

const auto path_to_cached_main_device = utils::project_folder() + "cache/main_device.json";
const auto path_to_pipeline_cache = utils::project_folder() + "cache/cache.packed";

#define VMA_DEFAULT_MEMORY_SIZE (32 * 1024 * 1024)

container::container() :
  instance(VK_NULL_HANDLE),
  debug_messenger(VK_NULL_HANDLE),
  physical_device(VK_NULL_HANDLE),
  device(VK_NULL_HANDLE),
  surface(VK_NULL_HANDLE),
  swapchain(VK_NULL_HANDLE),
  graphics_queue(VK_NULL_HANDLE),
  compute_queue(VK_NULL_HANDLE),
  presentation_queue(VK_NULL_HANDLE),
  cache(VK_NULL_HANDLE),
  graphics_command_pool(VK_NULL_HANDLE),
  transfer_command_pool(VK_NULL_HANDLE)
{
  load_dispatcher1();

  vk::ApplicationInfo app_info(DEVILS_ENGINE_PROJECT_NAME, 0, DEVILS_ENGINE_ENGINE_NAME, 0, VK_API_VERSION_1_0);

  if (enable_validation_layers && !check_validation_layer_support(default_validation_layers)) {
    utils::error("Could not find validation layers for Vulkan");
  }

  const auto req_extensions = get_required_extensions();

  vk::InstanceCreateInfo i({}, &app_info, default_validation_layers, req_extensions);
  
  auto ins = vk::createInstance(i);
  instance = ins;

  load_dispatcher2(instance);

  if (enable_validation_layers) {
    debug_messenger = create_debug_messenger(instance);
  }

  cached_system_data data DEVILS_ENGINE_PAINTER_CACHED_SYSTEM_DATA_INIT;
  if (file_io::exists(path_to_cached_main_device)) {
    do {
      const auto json = file_io::read(path_to_cached_main_device);
      
      const auto err = utils::from_json(data, json);
      if (err) {
        utils::warn("Could not parse cached device info '{}'. Trying to recreate info", path_to_cached_main_device);
        break;
      }

      const auto devices = ins.enumeratePhysicalDevices();
      for (const auto dev : devices) {
        const auto props = dev.getProperties();
        if (data.device_id == props.deviceID && data.vendor_id == props.vendorID) {
          physical_device = dev;
          break;
        }
      }

      if (physical_device == VK_NULL_HANDLE) utils::warn("Cached device info contains unknown device '{}' (id: '{}', vendor: '{}'). Trying to recreate info", data.device_name, data.device_id, data.vendor_id);
    } while (false);
  }

  if (physical_device == VK_NULL_HANDLE) {
    utils::info("Finding main presentation device...");
    physical_device = find_device_process(instance, &data);
  }

  if (physical_device == VK_NULL_HANDLE) utils::error("Could not find proper physical device for rendering");

  vk::PhysicalDevice dev(physical_device);
  const auto props = dev.getProperties();
  utils::info("Using physical device '{}' for rendering purposes", props.deviceName);

  // нужно еще задать фичи

  // создадим устройство, можно поди так же использовать мейкеры
  {
    device_maker dm(ins);
    device = dm.beginDevice(dev).setExtensions(default_device_extensions).createQueues(1).features(f).create(default_validation_layers, "Graphics device");
  }

  load_dispatcher3(device);

  vk::Device d(device);
  d.getQueue(data.graphics_queue, 1, &graphics_queue);
  d.getQueue(data.compute_queue, 1, &compute_queue);
  d.getQueue(data.present_queue, 1, &presentation_queue);

  {
    std::vector<uint8_t> cache_data;
    if (file_io::exists(path_to_pipeline_cache)) {
      const auto raw = file_io::read<uint8_t>(path_to_pipeline_cache);
      cache_data = utils::decompress(raw);
    }

    if (cache_data.empty()) {
      utils::info("Couldnt load old cache file '{}'. Creating one from scratch", path_to_pipeline_cache);
    }
    cache = d.createPipelineCache(vk::PipelineCacheCreateInfo(vk::PipelineCacheCreateFlagBits::eExternallySynchronized, cache_data.size(), cache_data.data()));
  }

  // тут нужно создать аллокаторы
  { 
    const auto f = make_functions();
    vma::AllocatorCreateInfo i({}, physical_device, device, VMA_DEFAULT_MEMORY_SIZE);
    i.instance = instance;
    i.pVulkanFunctions = &f;
    i.vulkanApiVersion = VK_API_VERSION_1_0;

    buffer_allocator = vma::createAllocator(i);
  }

  {
    descriptor_pool_maker dpm(d);

    descriptor_pool = 
      dpm.poolSize(vk::DescriptorType::eStorageBuffer, 20)
          .poolSize(vk::DescriptorType::eUniformBuffer, 10)
          .poolSize(vk::DescriptorType::eStorageImage, 10)
          .poolSize(vk::DescriptorType::eCombinedImageSampler, 10)
          .create("default_descriptor_pool");
  }

  {
    const vk::CommandPoolCreateInfo ci(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient,
      data.graphics_queue
    );
    auto pool = d.createCommandPool(ci);
    graphics_command_pool = pool;
    set_name(d, pool, "graphics command pool");
  }
      
  {
    const vk::CommandPoolCreateInfo ci(
      vk::CommandPoolCreateFlagBits::eTransient,
      data.graphics_queue
    );
    auto pool = d.createCommandPool(ci);
    transfer_command_pool = pool;
    set_name(d, pool, "transfer command pool");
  }

  {
    const vk::FenceCreateInfo i;
    transfer_fence = d.createFence(i);
  }
}

container::~container() noexcept {
  auto d = vk::Device(device);
  d.waitIdle();

  flush_cache();

  d.destroy(transfer_fence);
  d.destroy(transfer_command_pool);
  d.destroy(graphics_command_pool);
  d.destroy(descriptor_pool);
  d.destroy(cache);
  vma::Allocator(buffer_allocator).destroy();
  d.destroy();
  destroy_debug_messenger(instance, debug_messenger);
  vk::Instance(instance).destroy();
}

void container::flush_cache() {
  const auto data = vk::Device(device).getPipelineCacheData(cache);
  const auto compressed_data = utils::compress(data, utils::compression_level::best);
  file_io::write(compressed_data, path_to_pipeline_cache);
}

}
}