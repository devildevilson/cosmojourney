#ifndef DEVILS_ENGINE_PAINTER_STAGE_H
#define DEVILS_ENGINE_PAINTER_STAGE_H

#include <cstddef>
#include <cstdint>
#include <array>
#include "utils/list.h"
#include "vulkan_minimal.h"

namespace devils_engine {
namespace painter {

namespace stage_list_type {
enum values {
  siblings,
  count
};
}

// лучше навеное не через дабл лист, а просто друг за другом расположить
// а еще и походу лист нужно указать только в некоторых стейджах
class stage { // public utils::forw::list<stage, stage_list_type::siblings>
public:
  virtual ~stage() noexcept = default;

  virtual void begin(void* context) = 0;
  virtual void process(void* context) = 0; // какой то минимальный контекст
  virtual void clear() = 0;

  virtual void* pick_render_pass() const;

  stage* next() const;
protected:
  // нужно как то найти например рендерпасс
  // хотя если это единственное что нужно найти то мех
  stage* parent;
  
  // повторяется примерно повсеместно
  void* pipeline_layout;
  void* pipeline;
  void* shaders; // это наверное будут ресурсы
  void* descriptors;

  // возможно было бы еще неплохо сделать копирование получаемого изображения после рендера в текущем стейдже
  // 
};

// тут мы должны стартануть кью
// и передать нужные данные в следующий
class queue_stage {
public:
  virtual void begin(void* context) = 0;
  virtual void process(void* context) = 0; // какой то минимальный контекст
  virtual void clear() = 0;
protected:
  VkDevice device;
  VkQueue queue;
  VkSemaphore semaphore; // сколько сигналок? предположительно их может быть несколько
  std::array<VkSemaphore, 16> wait_semaphores;

  stage* childs;
};

// для каждого рендер пасса свой инпут по идее
// как сделать простейший рендер_таргет? по идее это просто фреймбуфер и данные с него
// не всегда, в базовом уровне это набор ИмейджВью + заданы начальные и конечные состояния изображения
// настройки пиплайна меняются соответственно, 
class render_pass_stage {
public:

protected:
  VkDevice device;
  VkRenderPass pass;
  void* render_taget;
};

class event_stage {
public:

  // чекаем доступность или ждем
protected:
  VkDevice device;
  VkEvent event;
};
}
}

#endif