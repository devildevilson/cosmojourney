#ifndef DEVILS_ENGINE_PAINTER_PRIMITIVES_H
#define DEVILS_ENGINE_PAINTER_PRIMITIVES_H

#include <cstddef>
#include <cstdint>
#include "utils/list.h"
#include "vulkan_minimal.h"
#include "pipeline_create_config.h"

namespace devils_engine {
namespace painter {

namespace primitive_list_type {
enum values {
  siblings,
  count
};
}

// какой то минимальный контекст
// в качестве контекста теперь будет только командный буфер
//struct context {
//  uint32_t index;
//  VkCommandBuffer buffer;
//};

// для двойной буферизации создадим 2 набора последовательностей stage
// 

class arbitrary_data {
public:
  virtual ~arbitrary_data() noexcept = default;
};

// порядок важен, но скорее всего и так и сяк придется в коде учитывать смену указателей
class stage : public arbitrary_data, public utils::forw::list<stage, primitive_list_type::siblings> {
public:
  virtual ~stage() noexcept = default;

  virtual void begin() = 0;
  virtual void process(VkCommandBuffer buffer) = 0; // ЖЕЛАТЕЛЬНО чтобы был конст
  virtual void clear() = 0;

  inline stage* next() const { return utils::forw::list_next<primitive_list_type::siblings>(this); }
  inline void set_next(stage* s) { utils::forw::list_add<primitive_list_type::siblings>(this, s); }
};

class parent_stage : public stage {
public:
  inline parent_stage() noexcept : childs(nullptr) {}
  virtual ~parent_stage() noexcept = default;
  inline void set_childs(stage* childs) { this->childs = childs; }
protected:
  stage* childs;
};

// порядок теперь супер важен: сначала свопчеин, потом аттачменты, и в конце фреймбуфер
class recreate_target : public arbitrary_data, public utils::ring::list<recreate_target, primitive_list_type::siblings> {
public:
  virtual ~recreate_target() noexcept = default;
  virtual void recreate(const uint32_t width, const uint32_t height) = 0;

  inline recreate_target* next(const recreate_target* ref) const { return utils::ring::list_next<primitive_list_type::siblings>(this, ref); }
  inline void set_next(recreate_target* s) { utils::ring::list_radd<primitive_list_type::siblings>(this, s); }
};

// порядок неважен
class recompile_shaders_target : public arbitrary_data, public utils::forw::list<recompile_shaders_target, primitive_list_type::siblings> {
public:
  virtual ~recompile_shaders_target() noexcept = default;
  // мы могли бы заранее скомпилировать шейдер модули
  virtual uint32_t recompile_shaders() = 0;

  inline recompile_shaders_target* next() const { return utils::forw::list_next<primitive_list_type::siblings>(this); }
  inline void set_next(recompile_shaders_target* s) { utils::forw::list_add<primitive_list_type::siblings>(this, s); }
};

struct semaphore_provider {
  VkSemaphore signal;
  uint32_t signal_stage;
  inline semaphore_provider() noexcept : signal(VK_NULL_HANDLE), signal_stage(0) {}
};

struct semaphore_resource : public semaphore_provider {
  VkDevice device;

  semaphore_resource(VkDevice device);
  ~semaphore_resource() noexcept;
};

// ожидание ожиданию рознь, так что бегать тут ничего не придется
class wait_target : public arbitrary_data {
public:
  virtual ~wait_target() noexcept = default;
  virtual uint32_t wait(const size_t max_time) const = 0;
};

struct wait_fence_provider : public wait_target {
  VkFence fence;
  inline wait_fence_provider() noexcept : fence(VK_NULL_HANDLE) {}
};

struct wait_event_provider : public wait_target {
  VkEvent event;
  inline wait_event_provider() noexcept : event(VK_NULL_HANDLE) {}
  // reset?
};

class submit_target : public arbitrary_data {
public:
  virtual ~submit_target() noexcept = default;
  virtual void submit() const = 0; // ??????
};

struct pipeline_layout_provider {
  VkPipelineLayout pipeline_layout;
  uint32_t pipeline_bind_point;
  inline pipeline_layout_provider() noexcept : pipeline_layout(VK_NULL_HANDLE), pipeline_bind_point(0) {}
};

struct pipeline_provider : public pipeline_layout_provider {
  VkPipeline pipeline;
  inline pipeline_provider() noexcept : pipeline(VK_NULL_HANDLE) {}
};

// он определенно должен взаимодействовать с аттачмент контейнером
// чтобы получить оттуда список вью
struct render_pass_provider {
  VkRenderPass render_pass;
  inline render_pass_provider() noexcept : render_pass(VK_NULL_HANDLE) {}
};

// для того чтобы привязать сет нужен VkPipelineLayout + bind point 
// надо бы разделить собственно пайп от лэйаута
struct descriptor_set_provider {
  VkDescriptorSet set;
  inline descriptor_set_provider() noexcept : set(VK_NULL_HANDLE) {}
};

struct vertex_draw_provider {
  uint32_t vertex_count;
  uint32_t instance_count;
  uint32_t first_vertex;
  uint32_t first_instance;
  inline vertex_draw_provider() noexcept : vertex_count(0), instance_count(0), first_vertex(0), first_instance(0) {}
};

struct indexed_draw_provider {
  uint32_t index_count;
  uint32_t instance_count;
  uint32_t first_index;
  int32_t vertex_offset;
  uint32_t first_instance;
  inline indexed_draw_provider() noexcept : index_count(0), instance_count(0), first_index(0), vertex_offset(0), first_instance(0) {}
};

struct buffer_provider {
  VkBuffer buffer;
  size_t offset;
  // размер?
  inline buffer_provider() noexcept : buffer(VK_NULL_HANDLE), offset(0) {}
};

// контейнеры для 2D картинок (в прочем все картинки мы можем свести к 2D)
// да и вообще к 1D если так подумать, но это филосовский вопрос
// имя у контейнера? полезно
class image_container {
public:
  struct extent_t { uint32_t width, height; };

  std::string container_name;

  inline image_container(std::string container_name) noexcept : container_name(std::move(container_name)) {}
  virtual ~image_container() noexcept = default;

  virtual bool is_exists(const uint32_t index) const = 0;
  virtual uint32_t create(std::string name, const extent_t extent, const uint32_t format, VkSampler sampler) = 0;
  virtual uint32_t create_any(std::string name, const extent_t extent, const uint32_t format, VkSampler sampler) = 0;
  virtual void destroy(const uint32_t index) = 0;

  virtual extent_t extent(const uint32_t index) const = 0;
  virtual uint32_t format(const uint32_t index) const = 0;
  virtual VkImage storage(const uint32_t index) const = 0;
  virtual VkImageView view(const uint32_t index) const = 0;
  virtual VkSampler sampler(const uint32_t index) const = 0;
  virtual std::string_view name(const uint32_t index) const = 0;

  virtual void update_descriptor_set(VkDescriptorSet set, const uint32_t binding, const uint32_t first_element) const = 0;
  virtual void change_layout(VkCommandBuffer buffer, const uint32_t index, const uint32_t old_layout, const uint32_t new_layout) const = 0;
  virtual void change_layout_all(VkCommandBuffer buffer, const uint32_t old_layout, const uint32_t new_layout) const = 0;
  virtual void copy_data(VkCommandBuffer buffer, VkImage image, const uint32_t index) const = 0;
  virtual void blit_data(VkCommandBuffer buffer, const std::tuple<VkImage,uint32_t,uint32_t> &src_image, const uint32_t index, const uint32_t filter = 0) const = 0;
};

// короче судя по всему до этого индекса ВООБЩЕ нет никому дела кроме фреймбуфера
class frame_acquisitor {
public:
  uint32_t max_images;
  uint32_t current_image_index;

  inline frame_acquisitor() noexcept : max_images(0), current_image_index(0) {}
  virtual ~frame_acquisitor() noexcept = default;
  virtual uint32_t acquire_next_image(size_t timeout, VkSemaphore semaphore, VkFence fence) = 0;
  virtual VkImage frame_storage(const uint32_t index) const = 0;
  virtual uint32_t frame_format(const uint32_t index) const = 0;
};

class attachments_provider {
public:
  size_t attachments_count;
  const attachment_config_t* attachments;
  uint32_t width, height;

  inline attachments_provider() noexcept : attachments_count(0), attachments(nullptr), width(0), height(0) {}
  virtual ~attachments_provider() noexcept = default;
  virtual size_t attachment_handles(const size_t buffering, VkImageView* views, const size_t max_size) const = 0;
};

class framebuffer_provider : public recreate_target {
public:
  const struct render_pass_provider* render_pass_provider;
  const class attachments_provider* attachments_provider;

  inline framebuffer_provider() noexcept : render_pass_provider(nullptr), attachments_provider(nullptr) {}
  virtual ~framebuffer_provider() noexcept = default;
  virtual VkFramebuffer current_framebuffer() const = 0;
};

class semaphore_wait_dependency {
public:
  virtual ~semaphore_wait_dependency() noexcept = default;
  virtual void add(VkSemaphore semaphore, const uint32_t stage_flag) = 0;
};

}
}

#endif