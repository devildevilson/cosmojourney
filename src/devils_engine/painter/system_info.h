#ifndef DEVILS_ENGINE_PAINTER_SYSTEM_INFO_H
#define DEVILS_ENGINE_PAINTER_SYSTEM_INFO_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "common.h"
#include "vulkan_minimal.h"

// было бы еще неплохо посмотреть какие фичи есть у устройства
// для этого нужно подтягивать структуры с фичами, но беда в том что это не энумы а говно какое то
// так что желательно пройтись по ним и например найти полный размер всех фич
// для этого можно пробежаться по всем структурам в рефлекте
// и даже найти по имени переменной фичу + также расставить значения в битовом поле

// часть данных из этой структуры запишем в кеш - по нему быстренько снова создадим устройство
// какие данные положим на диск?

namespace devils_engine {
namespace painter {
struct system_info {
  struct physical_device {
    enum class type {
      other,
      integrated_gpu,
      discrete_gpu,
      virtual_gpu,
      cpu,
      count
    };

    enum class present_mode {
      immediate,
      mailbox,
      fifo,
      fifo_relaxed,
      shared_demand_refresh,
      shared_continuous_refresh,
      count
    };

    struct queue_properties_t {
      VkFlags flags;
      uint32_t queue_count;
      uint32_t timestamp_valid_bits;
    };

    VkPhysicalDevice handle;
    std::string name;
    size_t memory;
    uint32_t id;
    uint32_t vendor_id;
    enum type type;
    uint32_t queue_family_index_surface_support;
    std::vector<queue_properties_t> queue_families;
    std::vector<present_mode> present_modes;

    physical_device();
  };

  VkInstance instance;
  std::vector<physical_device> devices;

  // да наверн сразу все и найдем че нам
  system_info();
  // не забыть перед этим удалить VkSurfaceKHR
  ~system_info();

  // запишем в массив
  void check_devices_surface_capability(const VkSurfaceKHR s);

  void dump_cache_to_disk();
};
}
}

#endif