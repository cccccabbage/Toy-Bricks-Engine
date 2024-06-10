#pragma once
#include "TBEngine/utils/includes/includeVulkan.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/base/vulkanAbstractBase.hpp"

namespace TBE::Graphics {

class RenderPass : public VulkanAbstractBase {
    using super = VulkanAbstractBase;

public:
    RenderPass() : super() {}
    ~RenderPass();

    void
    init(vk::Format swapchainFormat, vk::Format depthFormat, vk::SampleCountFlagBits msaaSamples);
    void destroy() override;

public:
    vk::RenderPass renderPass{};

private:
    bool inited = false;
};

} // namespace TBE::Graphics
