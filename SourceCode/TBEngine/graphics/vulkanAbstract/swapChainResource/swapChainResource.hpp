#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/base/vulkanAbstractBase.hpp"

#include <vector>
#include <utility>

namespace TBE::Graphics {
using Utils::Log::logErrorMsg;

class SwapchainResource : public VulkanAbstractBase {
public:
    SwapchainResource() = delete;
    SwapchainResource(const vk::Device&         device_,
                      const vk::PhysicalDevice& phyDevice_,
                      const vk::SurfaceKHR&     surface_)
        : VulkanAbstractBase(device_, phyDevice_), surface(surface_) {}
    ~SwapchainResource();

public:
    void init(const vk::PhysicalDevice& phyDevice, const std::pair<uint32_t, uint32_t>& bufferSize);
    void destroy() override;

    void createSwapChain(const std::pair<uint32_t, uint32_t>& bufferSize);
    void createImages();
    void createViews();

public:
    vk::SwapchainKHR           swapchain{};
    std::vector<vk::Image>     images{};
    std::vector<vk::ImageView> views{};

    vk::Format format{};

private:
    const vk::SurfaceKHR& surface;

private:
    bool swapchainInited = false;
    bool imageInited     = false;
    bool viewInited      = false;
};

} // namespace TBE::Graphics
