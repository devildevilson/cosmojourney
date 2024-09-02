#ifndef DEVILS_ENGINE_PAINTER_COMMON_STAGES_H
#define DEVILS_ENGINE_PAINTER_COMMON_STAGES_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include "primitives.h"

namespace devils_engine {
namespace painter {

class memory_barrier : public sibling_stage {
public:
  memory_barrier(const uint32_t srcAccessMask, const uint32_t dstAccessMask, const uint32_t srcStageMask, const uint32_t dstStageMask) noexcept;

  void begin() override;
  void process(VkCommandBuffer buffer) override;
  void clear() override;
protected:
  uint32_t srcAccessMask; 
  uint32_t dstAccessMask; 
  uint32_t srcStageMask; 
  uint32_t dstStageMask;
};

class compute_sync : public memory_barrier {
public:
  compute_sync() noexcept;
};

class compute_to_graphics_sync : public memory_barrier {
public:
  compute_to_graphics_sync() noexcept;
};

class set_event : public sibling_stage {
public:
  set_event(VkEvent event, const uint32_t stage_flags) noexcept;
  void begin() override;
  void process(VkCommandBuffer buffer) override;
  void clear() override;
protected:
  VkEvent event;
  uint32_t stage_flags;
};

class pipeline_view : public sibling_stage {
public:
  pipeline_view(const pipeline_provider* provider) noexcept;

  void begin() override;
  void process(VkCommandBuffer buffer) override;
  void clear() override;
protected:
  const pipeline_provider* provider;
};

class descriptor_set_view : public sibling_stage {
public:
  descriptor_set_view(const pipeline_provider* provider, const uint32_t first_set, std::vector<VkDescriptorSet> sets) noexcept;
  void begin() override;
  void process(VkCommandBuffer buffer) override;
  void clear() override;
protected:
  const pipeline_layout_provider* provider;
  uint32_t first_set;
  std::vector<VkDescriptorSet> sets;
};

// буферы можем тоже прибиндить сразу все, не нужно их перебиндить
// в будущем потребуется указать еще и офсет, наверное нужно будет сделать массив буфер провайдеров
class bind_vertex_buffer_view : public sibling_stage {
public:
  bind_vertex_buffer_view(const uint32_t first_buffer, std::vector<VkBuffer> buffers) noexcept;
  void begin() override;
  void process(VkCommandBuffer buffer) override;
  void clear() override;
protected:
  uint32_t first_buffer;
  std::vector<VkBuffer> buffers;
};

class bind_index_buffer_view : public sibling_stage {
public:
  bind_index_buffer_view(VkBuffer index, const size_t offset) noexcept;
  void begin() override;
  void process(VkCommandBuffer buffer) override;
  void clear() override;
protected:
  VkBuffer index;
  size_t offset;
};

class draw : public sibling_stage {
public:
  draw(const vertex_draw_provider* provider) noexcept;
  void begin() override;
  void process(VkCommandBuffer buffer) override;
  void clear() override;
protected:
  const vertex_draw_provider* provider;
};

class indexed_draw : public sibling_stage {
public:
  indexed_draw(const indexed_draw_provider* provider) noexcept;
  void begin() override;
  void process(VkCommandBuffer buffer) override;
  void clear() override;
protected:
  const indexed_draw_provider* provider;
};

class draw_indirect : public sibling_stage {
public:
  draw_indirect(VkBuffer indirect, const size_t offset) noexcept;
  void begin() override;
  void process(VkCommandBuffer buffer) override;
  void clear() override;
protected:
  VkBuffer indirect;
  size_t offset;
};

class indexed_draw_indirect : public sibling_stage {
public:
  indexed_draw_indirect(VkBuffer indirect, const size_t offset) noexcept;
  void begin() override;
  void process(VkCommandBuffer buffer) override;
  void clear() override;
protected:
  VkBuffer indirect;
  size_t offset;
};

}
}

#endif