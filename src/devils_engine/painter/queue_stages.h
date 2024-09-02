#ifndef DEVILS_ENGINE_PAINTER_QUEUE_STAGES_H
#define DEVILS_ENGINE_PAINTER_QUEUE_STAGES_H

#include <cstddef>
#include <cstdint>
#include "primitives.h"

// так вот что я понял: несколько командных буферов переданных кью
// вулкан будет пытаться их обработать максимально параллельно
// а вот внутри самого командного буфера параллельности никакой не будет 
// значит чтобы сделать что то более менее сложное мне нужно:
// последовательно задать буферы через сабмиты
// до этого собрать команды в него
// значит надо 250% вынести понятие буферизации из шагов рендера
// и создать основной рендер по количеству желаемой буферизации
// тут встает вопрос с ресурсами в самих шагах
// нам надо как то например обеспечить уникальность пиплинов
// стартовый класс будет хранить командный буфер а потом передавать только его дальше по иерархии

namespace devils_engine {
namespace painter {
class queue_dependant : public sibling_stage, public parent_stage, public submit_target, public semaphore_provider, public semaphore_wait_dependency {
public:
  constexpr static const size_t wait_semaphores_max = 16;

  queue_dependant(VkDevice device, VkCommandPool pool, VkQueue queue, std::initializer_list<VkSemaphore> wait_sems, std::initializer_list<uint32_t> wait_flags);
  ~queue_dependant() noexcept;

  void begin() override;
  void process(VkCommandBuffer) override;
  void clear() override;
  void submit() const override;
  void add(VkSemaphore semaphore, const uint32_t stage_flag) override;
private:
  VkDevice device;
  VkCommandPool pool;
  VkQueue queue;

  VkCommandBuffer buffer;

  size_t wait_semaphores_count;
  VkSemaphore wait_semaphores[wait_semaphores_max];
  uint32_t wait_flags[wait_semaphores_max];
};

class queue_main : public sibling_stage, public parent_stage, public submit_target, public semaphore_provider, public wait_fence_provider, public semaphore_wait_dependency {
public:
  constexpr static const size_t wait_semaphores_max = 16;

  queue_main(VkDevice device, VkCommandPool pool, VkQueue queue, std::initializer_list<VkSemaphore> wait_sems, std::initializer_list<uint32_t> wait_flags);
  ~queue_main() noexcept;

  void begin() override;
  void process(VkCommandBuffer) override;
  void clear() override;
  void submit() const override;
  void add(VkSemaphore semaphore, const uint32_t stage_flag) override;
private:
  VkDevice device;
  VkCommandPool pool;
  VkQueue queue;

  VkCommandBuffer buffer;

  size_t wait_semaphores_count;
  VkSemaphore wait_semaphores[wait_semaphores_max];
  uint32_t wait_flags[wait_semaphores_max];
};

// для того чтобы полностью запустить фрейм что нам нужно?
// 1) пробежать клир
// 2) получить новый индекс картинки
// 3) подождать фенс и другие вещи
// 4) пробежать бегин
// 5) пробежать process
// 6) запустить презентацию

class queue_present : public present_target, public semaphore_provider, public wait_fence_provider {
public:
  static const size_t wait_targets_max_count = 8;

  queue_present(VkDevice device, VkQueue queue, VkSwapchainKHR swapchain, frame_acquisitor* fram, queue_main* main);
  ~queue_present() noexcept;

  void begin() override;
  uint32_t acquire_next_image() override;
  void process() override;
  uint32_t present() const override;

  void add_waiter(wait_target* w);
protected:
  VkDevice device;
  VkQueue queue;
  VkSwapchainKHR swapchain;
  frame_acquisitor* fram;
  queue_main* main;

  uint32_t wait_count;
  wait_target* wait_targets[wait_targets_max_count];
};

}
}

#endif