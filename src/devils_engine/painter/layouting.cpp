#include "layouting.h"

#include "vulkan_header.h"
#include "makers.h"

namespace devils_engine {
namespace painter {

layouting::layouting(VkDevice device, const create_info &info) : device(device) {
  {
    sampler_maker sm(device);
    immutable_linear = sm.addressMode(vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat)
                         .anisotropy(VK_TRUE, 16)
                         .borderColor(vk::BorderColor::eFloatTransparentBlack)
                         .compareOp(VK_FALSE, vk::CompareOp::eNever)
                         .filter(vk::Filter::eLinear, vk::Filter::eLinear)
                         .lod(0.0f, 0.0f)
                         .mipmapMode(vk::SamplerMipmapMode::eLinear)
                         .create("linear_sampler");

    immutable_nearest = sm.addressMode(vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat)
                          .anisotropy(VK_TRUE, 16)
                          .borderColor(vk::BorderColor::eFloatTransparentBlack)
                          //.compareOp(VK_FALSE, vk::CompareOp::eNever)
                          .filter(vk::Filter::eNearest, vk::Filter::eNearest)
                          //.lod(0.0f, 0.0f)
                          .mipmapMode(vk::SamplerMipmapMode::eNearest)
                          .create("linear_sampler");
  }

  {
    descriptor_pool_maker dpm(device);
    pool = dpm.poolSize(vk::DescriptorType::eStorageBuffer, info.readonly_storage_buffers_count * 2 + info.storage_buffers_count * 2)
              .poolSize(vk::DescriptorType::eUniformBuffer, 10)
              .poolSize(vk::DescriptorType::eStorageImage, info.storage_images_count * 2)
              .poolSize(vk::DescriptorType::eCombinedImageSampler, info.combined_image_samplers_count * 2)
              .poolSize(vk::DescriptorType::eInputAttachment, info.input_attachments_count * 2)
              .create("default_descriptor_pool");
  }
  
  {
    std::vector<vk::Sampler> combined(info.combined_image_samplers_count, immutable_linear);
    descriptor_set_layout_maker dslm(device);
    set_layout = dslm.binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAll, 1)
                     .binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll, info.readonly_storage_buffers_count)
                     .combined(2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, combined)
                     .binding(3, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment, info.input_attachments_count)
                     .binding(4, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll, info.storage_buffers_count)
                     .binding(5, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eAll, info.storage_images_count)
                     .create("default_layout");
  }

  {
    pipeline_layout_maker plm(device);
    pipeline_layout = plm.addDescriptorLayout(set_layout).addPushConstRange(0, 128).create("default_pipeline_layout");
  }

  {
    descriptor_set_maker dsm(device);
    set = dsm.layout(set_layout).create(pool, "main_descriptor_set")[0];
  }
}

layouting::~layouting() noexcept {
  vk::Device d(device);

  d.destroy(pipeline_layout);
  d.destroy(set_layout);
  d.destroy(pool);
  d.destroy(immutable_nearest);
  d.destroy(immutable_linear);
}

}
}