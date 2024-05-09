#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/base/vulkanAbstractBase.hpp"
#include "TBEngine/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"

#include <vector>
#include <span>

namespace TBE::Graphics {

class Descriptor : public VulkanAbstractBase {
public:
    Descriptor() = delete;
    Descriptor(const vk::Device& device_, const vk::PhysicalDevice& phyDevice_)
        : VulkanAbstractBase(device_, phyDevice_) {}
    ~Descriptor();

    void destroy() override;

public:
    void initLayout(const std::span<const vk::DescriptorSetLayoutBinding>& bindings);
    void initPool(uint32_t maxSets, const std::span<vk::DescriptorPoolSize> poolSizes);
    void initSets(std::span<BufferResourceUniform> uniBuffers,
                  const vk::Sampler&               sampler,
                  const vk::ImageView&             sampleTarget);

public:
    vk::DescriptorPool             pool{};
    vk::DescriptorSetLayout        layout{};
    std::vector<vk::DescriptorSet> sets{};

private:
    bool poolInited   = false;
    bool layoutInited = false;
    bool setsInited   = false;
};

} // namespace TBE::Graphics
