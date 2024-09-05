#include "render_pass_resources.h"

#include "vulkan_header.h"
#include "makers.h"
#include "auxiliary.h"

namespace devils_engine {
namespace painter {

static vk::ImageLayout read_only_optimal(const uint32_t format) {
  return format_is_depth_or_stencil(format) ? vk::ImageLayout::eDepthStencilReadOnlyOptimal : vk::ImageLayout::eShaderReadOnlyOptimal;
}

static vk::ImageLayout layout_from_type_and_format(const subpass_attachment_type type, const uint32_t format, const vk::ImageLayout prev) {
  if (type == subpass_attachment_type::intended && format_is_depth_or_stencil(format)) {
    return vk::ImageLayout::eDepthStencilAttachmentOptimal;
  } else if (type == subpass_attachment_type::intended && format_is_color(format)) {
    return vk::ImageLayout::eColorAttachmentOptimal;
  } else if (type == subpass_attachment_type::input) {
    return read_only_optimal(format);
  } else if (type == subpass_attachment_type::preserve) {
    return prev;
  } else if (type == subpass_attachment_type::sampled) {
    return read_only_optimal(format);
  } else if (type == subpass_attachment_type::storage) {
    return vk::ImageLayout::eGeneral;
  }

  return prev;
}

static uint32_t combine_access_masks(const subpass_attachment_type type, const uint32_t format, const uint32_t prev_mask) {
  uint32_t mask = prev_mask;
  if (type == subpass_attachment_type::intended && format_is_depth_or_stencil(format)) {
    mask = mask | static_cast<uint32_t>(vk::AccessFlagBits2::eDepthStencilAttachmentWrite);
  } else if (type == subpass_attachment_type::intended && format_is_color(format)) {
    mask = mask | static_cast<uint32_t>(vk::AccessFlagBits2::eColorAttachmentWrite);
  } else if (type == subpass_attachment_type::input) {
    if (format_is_depth_or_stencil(format)) {
      mask = mask | static_cast<uint32_t>(vk::AccessFlagBits2::eDepthStencilAttachmentRead);
    } else {
      mask = mask | static_cast<uint32_t>(vk::AccessFlagBits2::eColorAttachmentRead);
    }
  } else if (type == subpass_attachment_type::preserve) {
    mask = mask | 0; // не трогаем?
  } else if (type == subpass_attachment_type::sampled) {
    mask = mask | static_cast<uint32_t>(vk::AccessFlagBits2::eShaderSampledRead);
  } else if (type == subpass_attachment_type::storage) {
    mask = mask | static_cast<uint32_t>(vk::AccessFlagBits2::eShaderStorageRead) | static_cast<uint32_t>(vk::AccessFlagBits2::eShaderStorageWrite);
  }
  
  return mask;
}

static vk::ClearValue make_clear_value(const subpass_attachment_type type, const uint32_t format) {
  return format_is_depth_or_stencil(format) ? vk::ClearValue(vk::ClearDepthStencilValue()) : vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f));
}

static vk::AttachmentLoadOp make_load_op(const attachment_operation op) {
  auto vop = vk::AttachmentLoadOp::eLoad;
  switch (op) {
    case attachment_operation::dont_care: vop = vk::AttachmentLoadOp::eDontCare; break;
    case attachment_operation::clear: vop = vk::AttachmentLoadOp::eClear; break;
    case attachment_operation::keep: vop = vk::AttachmentLoadOp::eLoad; break;
    default: break;
  }

  return vop;
}

static vk::AttachmentStoreOp make_store_op(const attachment_operation op) {
  auto vop = vk::AttachmentStoreOp::eStore;
  switch (op) {
    case attachment_operation::dont_care: vop = vk::AttachmentStoreOp::eDontCare; break;
    case attachment_operation::clear: vop = vk::AttachmentStoreOp::eDontCare; break;
    case attachment_operation::keep: vop = vk::AttachmentStoreOp::eStore; break;
    default: break;
  }

  return vop;
}

simple_render_pass::simple_render_pass(VkDevice device, const render_pass_data_t* create_data, const attachments_provider* provider) :
  device(device), create_data(create_data), provider(provider)
{
  //create_render_pass();
}

simple_render_pass::~simple_render_pass() noexcept {
  if (render_pass != VK_NULL_HANDLE) vk::Device(device).destroy(render_pass);
  render_pass = VK_NULL_HANDLE;
}

void simple_render_pass::create_render_pass() {
  load_ops loads(provider->attachments_count, attachment_operation::keep);
  store_ops stores(provider->attachments_count, attachment_operation::keep);
  create_render_pass_raw(loads, stores);
}

void simple_render_pass::create_render_pass_raw(const load_ops &load, const store_ops &store) {
  render_pass_maker rpm(device);

  // эту фигню в настройках пасса указать надо
  for (size_t i = 0; i < provider->attachments_count; ++i) {
    rpm.attachmentBegin(static_cast<vk::Format>(provider->attachments[i].format));
    rpm.attachmentInitialLayout(vk::ImageLayout::eUndefined);
    rpm.attachmentFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    rpm.attachmentLoadOp(make_load_op(load[i])); // если это первый рендер пасс, то нам бы его чистить
    rpm.attachmentStoreOp(make_store_op(store[i]));
    rpm.attachmentStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    rpm.attachmentStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
  }

  std::vector<std::tuple<uint32_t, uint32_t>> access_masks(create_data->subpasses.size(), std::make_tuple(0,0));
  std::vector<std::tuple<uint32_t, uint32_t>> stage_masks(create_data->subpasses.size(), std::make_tuple(0,0));

  for (size_t s = 0; s < create_data->subpasses.size()-1; ++s) {
    auto &subpass_data = create_data->subpasses[s];
    auto [src_access, dst_access] = access_masks[s];
    auto [src_stage, dst_stage] = stage_masks[s];

    if (s == 0) {
      src_access = static_cast<uint32_t>(vk::AccessFlagBits::eNone);
      src_stage = static_cast<uint32_t>(vk::PipelineStageFlagBits2::eTopOfPipe);
    } else {
      auto [prev_src_access, prev_dst_access] = access_masks[s-1];
      auto [prev_src_stage, prev_dst_stage] = stage_masks[s-1];
      src_access = prev_dst_access;
      src_stage = prev_dst_stage;
    }

    for (size_t i = 0; i < subpass_data.attachments.size(); ++i) {
      utils_assertf(provider->attachments_count == subpass_data.attachments.size(), "Subpass attachments size {} != {} current attachments size", subpass_data.attachments.size(), provider->attachments_count);
      const auto type = subpass_data.attachments[i].type;
      const auto format = provider->attachments[i].format;

      rpm.subpassBegin();
      if (type == subpass_attachment_type::intended && format_is_depth_or_stencil(format)) {
        rpm.subpassDepthStencilAttachment(i, vk::ImageLayout::eDepthStencilAttachmentOptimal);
      } else if (type == subpass_attachment_type::intended && format_is_color(format)) {
        rpm.subpassColorAttachment(i, vk::ImageLayout::eColorAttachmentOptimal);
      } else if (type == subpass_attachment_type::input) {
        rpm.subpassInputAttachment(i, read_only_optimal(format));
      } else if (type == subpass_attachment_type::preserve) {
        rpm.addPreservedAttachmentIndex(i);
      } else if (type == subpass_attachment_type::sampled) {
        rpm.subpassInputAttachment(i, read_only_optimal(format));
      } else if (type == subpass_attachment_type::storage) {
        rpm.subpassInputAttachment(i, vk::ImageLayout::eGeneral);
      }

      dst_access = combine_access_masks(type, format, dst_access);
    }

    dst_stage = static_cast<uint32_t>(vk::PipelineStageFlagBits2::eAllGraphics);
    //dst_stage = static_cast<uint32_t>(vk::PipelineStageFlagBits2::eTopOfPipe);

    access_masks[s] = std::make_tuple(src_access, dst_access);
    stage_masks[s] = std::make_tuple(src_stage, dst_stage);
  }

  {
    auto [prev_src_access, prev_dst_access] = access_masks[access_masks.size()-2];
    auto [prev_src_stage, prev_dst_stage] = stage_masks[stage_masks.size()-2];
    const uint32_t src_access = prev_dst_access;
    const uint32_t src_stage = prev_dst_stage;
    uint32_t dst_access = 0;
    //uint32_t dst_stage = static_cast<uint32_t>(vk::PipelineStageFlagBits2::eAllGraphics);
    uint32_t dst_stage = static_cast<uint32_t>(vk::PipelineStageFlagBits2::eBottomOfPipe);

    const auto &arr = create_data->subpasses.back().attachments;
    for (size_t i = 0; i < arr.size(); ++i) {
      const auto type = arr[i].type;
      const auto format = provider->attachments[i].format;
      dst_access = combine_access_masks(type, format, dst_access);
    }

    dst_access = 0;
    access_masks.back() = std::make_tuple(src_access, dst_access);
    stage_masks.back() = std::make_tuple(src_stage, dst_stage);
  }

  for (size_t s = 0; s < create_data->subpasses.size(); ++s) {
    //auto &arr = data.subpasses[s].attachments;
    auto [src_access, dst_access] = access_masks[s];
    auto [src_stage, dst_stage] = stage_masks[s];

    const auto prev_subpass = s == 0 ? VK_SUBPASS_EXTERNAL : s-1;
    const auto next_subpass = s == create_data->subpasses.size()-1 ? VK_SUBPASS_EXTERNAL : s;
    rpm.dependencyBegin(prev_subpass, next_subpass);
    rpm.dependencyDependencyFlags(vk::DependencyFlagBits::eByRegion);
    //rpm.dependencyDependencyFlags(vk::DependencyFlagBits(0));
    rpm.dependencySrcAccessMask(static_cast<vk::AccessFlags>(src_access));
    rpm.dependencySrcStageMask(static_cast<vk::PipelineStageFlags>(src_stage));
    rpm.dependencyDstAccessMask(static_cast<vk::AccessFlags>(dst_access));
    rpm.dependencyDstStageMask(static_cast<vk::PipelineStageFlags>(dst_stage));
  }

  render_pass = rpm.create("render_pass_main");
}

main_render_pass::main_render_pass(VkDevice device, const render_pass_data_t* create_data, const attachments_provider* attachments) :
  simple_render_pass(device, create_data, attachments)
{}

main_render_pass::~main_render_pass() noexcept {}

void main_render_pass::create_render_pass() {
  load_ops loads(provider->attachments_count, attachment_operation::clear);
  store_ops stores(provider->attachments_count, attachment_operation::keep);
  create_render_pass_raw(loads, stores);
}

}
}