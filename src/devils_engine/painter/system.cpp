#include "system.h"

#include "vulkan_header.h"
#include "container.h"

namespace devils_engine {
namespace painter {

system::system() {}
system::~system() noexcept {}

uint32_t system::recompile_shaders() {
  uint32_t return_code = 0;
  for (auto p = recompiling_shaders; p != nullptr; p = p->next()) {
    const auto ret = p->recompile_shaders();
    return_code = std::max(ret, return_code);
  }

  return return_code;
}

void system::recreate(const uint32_t width, const uint32_t height) {
  for (auto p = recreating; p != nullptr; p = p->next(recreating)) {
    p->recreate(width, height);
  }
}

uint32_t system::compute_frame() {
  submiter_counter = (submiter_counter + 1) % frames_presenters.size();
  auto ptr = frames_presenters[submiter_counter].get();

  ptr->begin();

  // попытаемся получить изображение, если eErrorOutOfDateKHR то пересоздадим свопчеин и еще раз попробуем взять картинку
  auto res = vk::Result::eSuccess;
  do {
    res = (vk::Result)ptr->acquire_next_image();
    switch (res) {
      case vk::Result::eSuccess:
      case vk::Result::eSuboptimalKHR:
        break;
      case vk::Result::eErrorOutOfDateKHR: {
        vk::PhysicalDevice pd(main_container->physical_device);
        const auto caps = pd.getSurfaceCapabilitiesKHR(surface);
        recreate(caps.currentExtent.width, caps.currentExtent.height);
        break;
      }
      default: utils::error("Acquiring image error '{}'", vk::to_string(vk::Result(res)));
    }
  } while (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR);

  ptr->process();

  const auto res = ptr->present();
  switch (vk::Result(res)) {
    case vk::Result::eSuccess:
      break;
    case vk::Result::eSuboptimalKHR:
    case vk::Result::eErrorOutOfDateKHR: {
      // chatgpt говорит что мы можем получить caps.currentExtent.width == UINT32_MAX 
      // и че делать? брать масимальные значения? ... как будто нет
      vk::PhysicalDevice pd(main_container->physical_device);
      const auto caps = pd.getSurfaceCapabilitiesKHR(surface);
      recreate(caps.currentExtent.width, caps.currentExtent.height);
      break;
    }
    default: utils::error("Presentation error '{}'", vk::to_string(vk::Result(res)));
  }

  return submiter_counter;
}

VkInstance system::get_instance() const { return main_container->instance; }
VkDevice system::get_device() const { return main_container->device; }
container* system::get_main_container() const { return main_container.get(); }
void system::set_window_surface(VkSurfaceKHR surface) {
  this->surface = surface;
}

}
}