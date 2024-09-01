#include "queue_stages.h"

#include "vulkan_header.h"
#include "utils/core.h"

namespace devils_engine {
namespace painter {
queue_dependant::queue_dependant(VkDevice device, VkCommandPool pool, VkQueue queue, std::initializer_list<VkSemaphore> wait_sems, std::initializer_list<uint32_t> wait_flags) : 
  device(device), pool(pool), queue(queue), buffer(VK_NULL_HANDLE), wait_semaphores_count(0), wait_semaphores{}, wait_flags{0}
{
  vk::Device d(device);
  if (wait_sems.size() != wait_flags.size()) utils::error("Wrong number on wait data provided: {} != {}", wait_sems.size(), wait_flags.size());
  if (wait_sems.size() > wait_semaphores_max) utils::error("Too many wait data provided: {} > 16", wait_sems.size());

  vk::CommandBufferAllocateInfo cinf(pool, vk::CommandBufferLevel::ePrimary, 1);
  buffer = d.allocateCommandBuffers(cinf)[0];

  vk::SemaphoreCreateInfo sinf;
  signal = d.createSemaphore(sinf);
  signal_stage = static_cast<uint32_t>(vk::PipelineStageFlagBits::eBottomOfPipe);

  wait_semaphores_count = wait_sems.size();
  memcpy(wait_semaphores, wait_sems.begin(), wait_sems.size() * sizeof(*wait_sems.begin()));
  memcpy(this->wait_flags, wait_flags.begin(), wait_flags.size() * sizeof(*wait_flags.begin()));
}

queue_dependant::~queue_dependant() noexcept {
  vk::Device d(device);
  vk::CommandBuffer b(buffer);
  d.freeCommandBuffers(pool, b);

  d.destroy(signal);
}

void queue_dependant::begin() {
  vk::CommandBuffer b(buffer);
  b.reset();

  for (auto p = childs; p != nullptr; p = p->next()) { p->begin(); }
}

void queue_dependant::process(VkCommandBuffer) {
  for (auto p = childs; p != nullptr; p = p->next()) { p->process(buffer); }
}

void queue_dependant::clear() {
  for (auto p = childs; p != nullptr; p = p->next()) { p->clear(); }
}

void queue_dependant::submit() const {
  static_assert(sizeof(vk::PipelineStageFlags) == sizeof(uint32_t));
  const vk::Queue q(queue);
  const vk::Semaphore s(signal);
  const vk::CommandBuffer b(buffer);
  const vk::SubmitInfo inf(wait_semaphores_count, (vk::Semaphore*)wait_semaphores, (vk::PipelineStageFlags*)wait_flags, 1, &b, 1, &s);
  q.submit(inf);
}

void queue_dependant::add(VkSemaphore semaphore, const uint32_t stage_flag) {
  if (wait_semaphores_count >= wait_semaphores_max) utils::error("Too many wait semaphores");
  const size_t index = wait_semaphores_count;
  wait_semaphores_count += 1;
  wait_semaphores[index] = semaphore;
  wait_flags[index] = stage_flag;
}

queue_main::queue_main(VkDevice device, VkCommandPool pool, VkQueue queue, std::initializer_list<VkSemaphore> wait_sems, std::initializer_list<uint32_t> wait_flags) :
  device(device), pool(pool), queue(queue), buffer(VK_NULL_HANDLE), wait_semaphores_count(0), wait_semaphores{}, wait_flags{0}
{
  vk::Device d(device);
  if (wait_sems.size() != wait_flags.size()) utils::error("Wrong number on wait data provided: {} != {}", wait_sems.size(), wait_flags.size());
  if (wait_sems.size() > wait_semaphores_max) utils::error("Too many wait data provided: {} > 16", wait_sems.size());

  vk::CommandBufferAllocateInfo cinf(pool, vk::CommandBufferLevel::ePrimary, 1);
  buffer = d.allocateCommandBuffers(cinf)[0];

  vk::SemaphoreCreateInfo sinf;
  signal = d.createSemaphore(sinf);
  signal_stage = static_cast<uint32_t>(vk::PipelineStageFlagBits::eBottomOfPipe);

  vk::FenceCreateInfo finf(vk::FenceCreateFlagBits::eSignaled);
  fence = d.createFence(finf);

  wait_semaphores_count = wait_sems.size();
  memcpy(wait_semaphores, wait_sems.begin(), wait_sems.size() * sizeof(*wait_sems.begin()));
  memcpy(this->wait_flags, wait_flags.begin(), wait_flags.size() * sizeof(*wait_flags.begin()));
}

queue_main::~queue_main() noexcept {
  vk::Device d(device);
  vk::CommandBuffer b(buffer);
  d.freeCommandBuffers(pool, b);

  d.destroy(signal);
  d.destroy(fence);
}

void queue_main::begin() {
  vk::CommandBuffer b(buffer);
  b.reset();

  for (auto p = childs; p != nullptr; p = p->next()) { p->begin(); }
}

void queue_main::process(VkCommandBuffer) {
  for (auto p = childs; p != nullptr; p = p->next()) { p->process(buffer); }
}

void queue_main::clear() {
  for (auto p = childs; p != nullptr; p = p->next()) { p->clear(); }
}

void queue_main::submit() const {
  static_assert(sizeof(vk::PipelineStageFlags) == sizeof(uint32_t));
  vk::Fence f(fence);
  vk::Device d(device);
  d.resetFences(f);

  const vk::Queue q(queue);
  const vk::Semaphore s(signal);
  const vk::CommandBuffer b(buffer);
  const vk::SubmitInfo inf(wait_semaphores_count, (vk::Semaphore*)wait_semaphores, (vk::PipelineStageFlags*)wait_flags, 1, &b, 1, &s);
  q.submit(inf, f);
}

void queue_main::add(VkSemaphore semaphore, const uint32_t stage_flag) {
  if (wait_semaphores_count >= wait_semaphores_max) utils::error("Too many wait semaphores");
  const size_t index = wait_semaphores_count;
  wait_semaphores_count += 1;
  wait_semaphores[index] = semaphore;
  wait_flags[index] = stage_flag;
}



}
}