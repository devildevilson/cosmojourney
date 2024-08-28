#ifndef DEVILS_ENGINE_PAINTER_TARGET_H
#define DEVILS_ENGINE_PAINTER_TARGET_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include "vulkan_minimal.h"

// рендер таргет с точки зрения вулкана это набор ресурсов которые подключены в инпут атачмент
// потенциально таргетом может быть любая конфигурация из картинок
// что бы я хотел тут увидеть: минимальный набор структур для описания рендер таргета
// такой что мы потенциально можем его передать в создание рендер пасса
// и это покрое львиную долю того что нужно указать в рендер пассе
// во вторых необходимые данные для того чтобы пересоздать таргет при изменении окна
// в третьих сгенерить доп данные для шейдеров

// имеет смысл делать аттачмент глубины первым в списке, а потом все остальные
// либо первым либо последним

// что я вообще пытаюсь получить? я бы хотел вещь которую я могу более менее легко подменить в зависимости от использования
// ранее у меня были проблемы с вопросом откуда это все брать
// например у меня для 3й буферизации есть 3 командных буфера, 3 фреймбуфера, и далее
// это все по идее разные вещи которые могут не пересекаться
// имеет смысл например отделить таргет от команд
// таргет можно хранить просто где то около рендер пасса
// и так можно будет легко сделать например 2 паса друг за другом

typedef struct GLFWwindow GLFWwindow;

namespace devils_engine {
namespace painter {
// таргет должен хранить минимально необходимое для рендерпасса, а значит никаких аллокаций и сырых картинок
// это все будет создаваться отдельно, затем передаваться сюда чтобы заполнить структуры
class target {
public:
  struct attachment_state {
    VmaAllocation alloc;
    VkImageView view;
    VkImage image;
    uint32_t desired_initial_layout;
    uint32_t layout_after;
    uint32_t format;
  };

  struct part {
    attachment_state* atachments;
    uint32_t atachments_count;

    VkFramebuffer framebuffer;
    VkSemaphore wait_image;
    VkSemaphore finish_rendering;
    VkFence wait_frame;
  };

  // все равно должен быть способ получить все эти данные без виртуалок
  uint32_t image_index;
  part* parts;
  uint32_t parts_count;

  inline target() noexcept : image_index(0), parts(nullptr), parts_count(0) {}
  virtual ~target() noexcept = default;

  virtual void recreate(const uint32_t width, const uint32_t height) = 0;
  virtual uint32_t acquire_next_image() = 0;
  virtual bool wait(const size_t max_time) = 0;
  virtual uint32_t present() = 0;

  inline VkFramebuffer current_framebuffer() const { return parts[image_index].framebuffer; }
  inline VkSemaphore current_wait_semaphore() const { return parts[image_index].wait_image; }
  inline VkSemaphore current_finish_semaphore() const { return parts[image_index].finish_rendering; }
  inline VkFence current_fence() const { return parts[image_index].wait_frame; }
};

// подходим плавно к такой большой штуке как рендеринг контекст
struct target2 {
  struct attachment_state {
    VmaAllocation alloc;
    VkImageView view;
    VkImage image;
    uint32_t desired_initial_layout;
    uint32_t layout_after;
    uint32_t format;
  };

  std::vector<attachment_state> attachments;
  VkFramebuffer framebuffer;
  VkSemaphore wait_image;
  VkSemaphore finish_rendering;
  VkFence wait_frame;

  // вьюпорт + скиссор, значения по умолчанию для аттачментов
  // дескриптор будет наверное только один или два, и он будет задан в самом начале отдельно
  // буферы ? буферы задаются либо через сет, либо как вертексы в момент биндинга пайпа
  // как будто иерархия должна быть примерно такой:
  // кью -> дескрипторы (?) -> рендерпасс (или его отсутствие) -> вершины буферы -> пиплин -> дравколл
  // причем дескрипторы могут быть в любом месте (да и буферы тоже)
};

struct context {
  uint32_t index;
  VkCommandBuffer buffer;

};

struct attachment {
  VmaAllocation alloc;
  VkImageView view;
  VkImage image;
  uint32_t format;
};

// у всех изображений для фреймбуфера КРАЙНЕ ЖЕЛАТЕЛЬНО один и тот же размер (по крайней мере он должен быть больше чем размер области рендеринга)
struct attachment_container {
  std::vector<attachment> attachments;
  uint32_t width, height;
};

// прямо тут окно создадим? нам бы к нему доступ иметь где нибудь в другом месте, где?
// настройки для таргета какие?
class swapchain_target : public target {
public:
  swapchain_target(VkInstance instance, VkDevice device, GLFWwindow* window);
  ~swapchain_target() noexcept;

  void recreate(const uint32_t width, const uint32_t height) override;
  uint32_t acquire_next_image() override;
  bool wait(const size_t max_time) override;
  uint32_t present() override;
protected:
  VkInstance instance;
  VkDevice device;
  GLFWwindow* window;
  VkSwapchainKHR handle;
  VkSurfaceKHR surface;

  // нужно запомнить тут поелзные данные
  
  

  std::vector<target::part> parts_arr;
  std::vector<target::attachment_state> attachments_arr;
};
}
}

#endif