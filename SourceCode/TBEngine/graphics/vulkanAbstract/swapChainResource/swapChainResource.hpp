#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/base/base.hpp"

#include <vector>
#include <utility>

namespace TBE::Graphics {
using Utils::Log::logErrorMsg;

class SwapchainResource : public VulkanAbstractBase {
    // 1452
public:
    SwapchainResource() = default;
    ~SwapchainResource();

public:
    void initAll(const vk::Device*                    pDevice_,
                 vk::SurfaceKHR*                      pSurface_,
                 const vk::PhysicalDevice&            phyDevice,
                 const std::pair<uint32_t, uint32_t>& bufferSize);
    void destroy() override;

    void setPSurface(const vk::SurfaceKHR* pSurface_) { pSurface = pSurface_; };

    void createSwapChain(const vk::PhysicalDevice&            phyDevice,
                         const std::pair<uint32_t, uint32_t>& bufferSize);
    void createImages();
    void createViews();

public:
    vk::SwapchainKHR           swapchain{};
    std::vector<vk::Image>     images{};
    std::vector<vk::ImageView> views{};

    vk::Format format{};

private:
    const vk::SurfaceKHR* pSurface;
};

} // namespace TBE::Graphics