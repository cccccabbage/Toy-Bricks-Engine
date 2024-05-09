#pragma once
#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/base/vulkanAbstractBase.hpp"

namespace TBE::Graphics {

class RenderPass : public VulkanAbstractBase {
public:
    RenderPass() = delete;
    RenderPass(const vk::Device& device_, const vk::PhysicalDevice& phyDevice_)
        : VulkanAbstractBase(device_, phyDevice_) {}
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
