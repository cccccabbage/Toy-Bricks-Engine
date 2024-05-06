#pragma once
#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/base/base.hpp"

namespace TBE::Graphics {

class RenderPass : public VulkanAbstractBase {
public:
    RenderPass() = default;
    ~RenderPass();

    void initAll(const vk::Device*       pDevice_,
                 vk::Format              swapchainFormat,
                 vk::Format              depthFormat,
                 vk::SampleCountFlagBits msaaSamples);
    void destroy() override;

public:
    vk::RenderPass renderPass{};
};

} // namespace TBE::Graphics