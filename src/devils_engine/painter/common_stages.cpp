#include "common_stages.h"

#include "vulkan_header.h"

#undef MemoryBarrier

namespace devils_engine {
namespace painter {
memory_barrier::memory_barrier(const uint32_t srcAccessMask, const uint32_t dstAccessMask, const uint32_t srcStageMask, const uint32_t dstStageMask) noexcept :
  srcAccessMask(srcAccessMask), dstAccessMask(dstAccessMask), srcStageMask(srcStageMask), dstStageMask(dstStageMask)
{}

void memory_barrier::begin() {}
void memory_barrier::process(VkCommandBuffer buffer) {
  vk::CommandBuffer b(buffer);

  vk::MemoryBarrier2 bar(static_cast<vk::PipelineStageFlags2>(srcStageMask), static_cast<vk::AccessFlags2>(srcAccessMask), static_cast<vk::PipelineStageFlags2>(dstStageMask), static_cast<vk::AccessFlags2>(dstAccessMask));
  vk::DependencyInfo di(vk::DependencyFlagBits::eByRegion, bar);
  b.pipelineBarrier2(di);
}

void memory_barrier::clear() {}

compute_sync::compute_sync() noexcept : memory_barrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT) {}
compute_to_graphics_sync::compute_to_graphics_sync() noexcept : memory_barrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT) {}

pipeline_view::pipeline_view(const pipeline_provider* provider) noexcept : provider(provider) {}
void pipeline_view::begin() {}
void pipeline_view::process(VkCommandBuffer buffer) { 
  vk::CommandBuffer b(buffer); 
  b.bindPipeline(static_cast<vk::PipelineBindPoint>(provider->pipeline_bind_point), provider->pipeline);
}
void pipeline_view::clear() {}

descriptor_set_view::descriptor_set_view(const pipeline_provider* provider, const uint32_t first_set, std::vector<VkDescriptorSet> sets) noexcept : provider(provider), first_set(first_set), sets(std::move(sets)) {}
void descriptor_set_view::begin() {}
void descriptor_set_view::process(VkCommandBuffer buffer) {
  vk::CommandBuffer b(buffer); 
  b.bindDescriptorSets(static_cast<vk::PipelineBindPoint>(provider->pipeline_bind_point), provider->pipeline_layout, 0, sets.size(), (vk::DescriptorSet*)sets.data(), 0, nullptr);
}

void descriptor_set_view::clear() {}

// буферы можно прибиндить все с самого начала
// предпочтительно
bind_vertex_buffer_view::bind_vertex_buffer_view(const uint32_t first_buffer, std::vector<VkBuffer> buffers) noexcept : first_buffer(first_buffer), buffers(std::move(buffers)) {}
void bind_vertex_buffer_view::begin() {}
void bind_vertex_buffer_view::process(VkCommandBuffer buffer) {
  vk::CommandBuffer b(buffer);
  b.bindVertexBuffers(first_buffer, buffers.size(), (vk::Buffer*)buffers.data(), 0, nullptr);
}
void bind_vertex_buffer_view::clear() {}

bind_index_buffer_view::bind_index_buffer_view(VkBuffer index, const size_t offset) noexcept : index(index), offset(offset) {}
void bind_index_buffer_view::begin() {}
void bind_index_buffer_view::process(VkCommandBuffer buffer) {
  vk::CommandBuffer b(buffer);
  b.bindIndexBuffer(index, offset, vk::IndexType::eUint32);
}
void bind_index_buffer_view::clear() {}

draw::draw(const vertex_draw_provider* provider) noexcept : provider(provider) {}
void draw::begin() {}
void draw::process(VkCommandBuffer buffer) {
  vk::CommandBuffer b(buffer);
  b.draw(provider->vertex_count, provider->instance_count, provider->first_vertex, provider->first_instance);
}
void draw::clear() {}

indexed_draw::indexed_draw(const indexed_draw_provider* provider) noexcept : provider(provider) {}
void indexed_draw::begin() {}
void indexed_draw::process(VkCommandBuffer buffer) {
  vk::CommandBuffer b(buffer);
  b.drawIndexed(provider->index_count, provider->instance_count, provider->first_index, provider->vertex_offset, provider->first_instance);
}
void indexed_draw::clear() {}

draw_indirect::draw_indirect(VkBuffer indirect, const size_t offset) noexcept : indirect(indirect), offset(offset) {}
void draw_indirect::begin() {}
void draw_indirect::process(VkCommandBuffer buffer) {
  vk::CommandBuffer b(buffer);
  b.drawIndirect(indirect, offset, 1, sizeof(vk::DrawIndirectCommand));
}
void draw_indirect::clear() {}

indexed_draw_indirect::indexed_draw_indirect(VkBuffer indirect, const size_t offset) noexcept : indirect(indirect), offset(offset) {}
void indexed_draw_indirect::begin() {}
void indexed_draw_indirect::process(VkCommandBuffer buffer) {
  vk::CommandBuffer b(buffer);
  b.drawIndexedIndirect(indirect, offset, 1, sizeof(vk::DrawIndexedIndirectCommand));
}
void indexed_draw_indirect::clear() {}

}
}