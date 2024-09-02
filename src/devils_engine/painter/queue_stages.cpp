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

const size_t queue_present::wait_targets_max_count;
queue_present::queue_present(VkDevice device, VkQueue queue, VkSwapchainKHR swapchain, frame_acquisitor* fram, queue_main* main) :
  device(device), queue(queue), swapchain(swapchain), fram(fram), main(main)
{
  signal = vk::Device(device).createSemaphore(vk::SemaphoreCreateInfo());
  signal_stage = uint32_t(vk::PipelineStageFlagBits::eBottomOfPipe);
  fence = vk::Device(device).createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
}

queue_present::~queue_present() noexcept {
  vk::Device(device).destroy(signal);
  vk::Device(device).destroy(fence);
}

void queue_present::begin() {
  const size_t timeout = 1000ull * 1000ull * 1000ull; // 1 секунда

  vk::Device d(device);
  const auto res = d.waitForFences(vk::Fence(fence), VK_TRUE, timeout);
  if (res != vk::Result::eSuccess) utils::error("Wait for frame fence took too long. Error: {}", vk::to_string(res));
  d.resetFences(vk::Fence(fence));

  // 1) это пробежать клир
  main->clear();
}

uint32_t queue_present::acquire_next_image() {
  const size_t timeout = 1000ull * 1000ull * 1000ull; // 1 секунда
  return fram->acquire_next_image(timeout, signal, fence);
}

void queue_present::process() {
  const size_t timeout = 1000ull * 1000ull * 1000ull; // 1 секунда

  // 4) подождать фенс и другие вещи
  for (uint32_t i = 0; i < wait_count; ++i) {
    const auto res = wait_targets[i]->wait(timeout);
    if (res != 0) utils::error("Wait for too long. Error: {}", vk::to_string(vk::Result(res)));
  }

  // 2) это пробежать бегин 
  // нам с очень малой вероятностью потребуется знать какую картинку из свопчейна мы взяли
  main->begin();

  // 5) пробежать process
  main->process(VK_NULL_HANDLE);
}

uint32_t queue_present::present() const {
  VkSemaphore semaphores[2]{ signal, main->signal };
  vk::Result local_res = vk::Result::eSuccess;

  vk::PresentInfoKHR pi(2, (vk::Semaphore*)semaphores, 1, (vk::SwapchainKHR*)&swapchain, &fram->current_image_index, &local_res);
  const auto res = vk::Queue(queue).presentKHR(pi);
  return uint32_t(res);
}

void queue_present::add_waiter(wait_target* w) {
  if (wait_count >= wait_targets_max_count) utils::error("Too many wait targets");
  wait_targets[wait_count] = w;
  wait_count += 1;
}

}
}