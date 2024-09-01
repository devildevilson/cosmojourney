#ifndef DEVILS_ENGINE_PAINTER_SYSTEM_H
#define DEVILS_ENGINE_PAINTER_SYSTEM_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <parallel_hashmap/phmap.h>
#include "vulkan_minimal.h"
#include "primitives.h"
#include "pipeline_create_config.h"

namespace devils_engine {
namespace painter {

struct container;
struct attachments_container;
struct layouting;
class arbitrary_image_container; // sampler2D[]
class hierarchical_image_container; // sampler2D[]
class image_pool; // texture2DArray[]
// + этому надо сам сэмплер добавить
// или может быть все таки отказаться от семлеров в контейнерах вообще?
// мы можем отказаться от семплеров в контейнерах вообще и оставить только texture2D[] и texture2DArray[]
// так нам нужно будет определить откуда мы получаем картинку...
// хотя опять же у нас есть только четкое разделение на эррей и не эррей
// ВСЕ! теперь все контейнеры возвращают текстурки вида sampler2D
// осталось только примерно понять можно ли сделать в этих же контейнерах сторадж имейджы?

class simple_swapchain;

// тут что по идее создаем все ресурсы и даем несколкьо полезных функций наружу
// например зададим ряд интерфейсов и здесь их положим все в свои массивы
// выкинем наружу несколько функций которые пройдутся по интерфейсам и запустят функцию

// обязательно где то надо еще делать вот это vmaSetCurrentFrameIndex
class system {
public:
  struct system_create_info {
    // тут бы указать разные размеры всяких штук
  };

  system();
  ~system() noexcept;

  // как тут быть с конфигом? я бы так сказал: у нас должна быть возможность остановить эту систему 
  // удалить ее полностью и пересоздать и запустить вновь
  // должна ли загрузка происходить только в основном потоке? не обязательно
  // загрузка тащемта это набор функций которые выполняются одна за другой + синхронизация во время выполнения

  template <typename T, typename... Args>
  T *add_stage(Args&&... args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto p = ptr.get();
    stages.push_back(std::move(ptr));

    if constexpr (std::derived_from<T, recreate_target>) {
      if (recreating == nullptr) recreating = p;
      else recreating->add(p);
    } else if constexpr (std::derived_from<T, submit_target>) {
      if (submiting == nullptr) submiting = p;
      else submiting->add(p);
    } else if constexpr (std::derived_from<T, recompile_shaders_target>) {
      if (recompiling_shaders == nullptr) recompiling_shaders = p;
      else recompiling_shaders->add(p);
    }
    
    return p;
  }

  // количество нужно указать
  void create_layouting();

  void create_attachment_container(std::vector<attachment_config_t> config);



  VkInstance get_instance() const;
  VkDevice get_device() const;
  void set_window_surface(VkSurfaceKHR surface);

  graphics_pipeline_create_config* get_graphics_pipeline_config(const std::string &name);
  compute_pipeline_create_config* get_compute_pipeline_config(const std::string &name);
  render_pass_data_t* get_render_pass_config(const std::string &name);
  std::vector<attachment_config_t> & get_attachments_config(const std::string &name);
  sampler_config_t* get_sampler_config(const std::string &name);
  void dump_configs_to_disk() const;
  void reload_configs();

private:
  std::unique_ptr<container> main_container;

  std::unique_ptr<layouting> graphics_layout;
  std::unique_ptr<arbitrary_image_container> any_images;
  std::unique_ptr<hierarchical_image_container> block_images;
  std::unique_ptr<image_pool> array32_images;
  std::unique_ptr<image_pool> array512_images; // ?

  std::unique_ptr<attachments_container> attachments;
  // надо также выкинуть функции создания изображения
  
  // сюда нужно передать окно
  VkSurfaceKHR surface;
  std::unique_ptr<simple_swapchain> swapchain;
  
  // система должна подгрузить все конфиги всех вулкан штук и положить их где то здесь
  // + система подгрузит конфиги из кода + подгрузит конфиги с диска (диск по приоритету)
  phmap::flat_hash_map<std::string, graphics_pipeline_create_config> graphics_pipeline_configs;
  phmap::flat_hash_map<std::string, compute_pipeline_create_config> compute_pipeline_configs;
  phmap::flat_hash_map<std::string, render_pass_data_t> render_pass_configs;
  phmap::flat_hash_map<std::string, std::vector<attachment_config_t>> attachments_configs;
  phmap::flat_hash_map<std::string, sampler_config_t> sampler_configs;

  std::vector<std::unique_ptr<arbitrary_data>> stages;
  recreate_target* recreating;
  submit_target* submiting; // так указать это дело НЕЛЬЗЯ, желательно убрать из аттачмент контейнера swapchain
  recompile_shaders_target* recompiling_shaders; 

  std::vector<std::unique_ptr<submit_target>> frames_submitters;
  std::vector<std::unique_ptr<semaphore_resource>> frames_semaphores;
};
}
}

#endif