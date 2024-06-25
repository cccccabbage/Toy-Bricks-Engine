#pragma once

#include "TBEngine/utils/includes/includeVulkan.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/base/vulkanAbstractBase.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"

#include <vector>
#include <span>

namespace TBE::Graphics {

class Descriptor : public VulkanAbstractBase {
    using super = VulkanAbstractBase;

public:
    Descriptor() : super() {}
    ~Descriptor();

    void destroy() override;

public:
    void initLayout(const std::span<const vk::DescriptorSetLayoutBinding>& bindings);
    void initPool(uint32_t maxSets, const std::span<vk::DescriptorPoolSize> poolSizes);
    void initSets(const std::span<BufferResourceUniform> uniBuffers,
                  const vk::Sampler&                     sampler,
                  const vk::ImageView&                   sampleTarget);

public:
    vk::DescriptorPool             pool{};
    vk::DescriptorSetLayout        layout{};
    std::vector<vk::DescriptorSet> sets{};
};

} // namespace TBE::Graphics
