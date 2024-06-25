#include "swapchainResource.hpp"
#include "TBEngine/core/graphics/detail/graphicsDetail.hpp"
#include "TBEngine/core/graphics/graphics.hpp"

namespace TBE::Graphics {
using namespace TBE::Graphics::Detail;

SwapchainResource::SwapchainResource() : super(), surface(VulkanGraphics::surface) {
}

SwapchainResource::~SwapchainResource() {
    destroy();
}

void SwapchainResource::destroy() {
    static bool destroyed = false;
    if (!destroyed) {
        for (size_t i = 0; i < views.size(); i++) {
            device.destroy(views[i]);
        }
        device.destroy(swapchain);
        destroyed = true;
    }
}

void SwapchainResource::init(const vk::PhysicalDevice&            phyDevice,
                             const std::pair<uint32_t, uint32_t>& bufferSize) {
    createSwapChain(bufferSize);
    createImages();
    createViews();
}

void SwapchainResource::createSwapChain(const std::pair<uint32_t, uint32_t>& bufferSize) {
    auto swapChainSupport = SwapChainSupportDetails(phyDevice, surface);
    auto surfaceFormat    = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode      = chooseSwapPresentMode(swapChainSupport.presentModes);
    auto swapExtent       = chooseSwapExtent(swapChainSupport.capabilities, bufferSize);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.setSurface(surface)
        .setMinImageCount(imageCount)
        .setImageFormat(surfaceFormat.format)
        .setImageColorSpace(surfaceFormat.colorSpace)
        .setImageExtent(swapExtent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    auto                    indices            = QueueFamilyIndices(phyDevice, surface);
    std::array<uint32_t, 2> queueFamilyIndices = indices;

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndices);
    } else {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndices({}); // should this be a "{}" here, if I want to pass no indices?
    }

    createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(presentMode)
        .setClipped(vk::True) // discard pixels that are obsured
        .setOldSwapchain(nullptr);

    depackReturnValue(swapchain, device.createSwapchainKHR(createInfo));

    format = surfaceFormat.format;
}

void SwapchainResource::createImages() {
    depackReturnValue(images, device.getSwapchainImagesKHR(swapchain));
}

void SwapchainResource::createViews() {
    views.resize(images.size());
    for (size_t i = 0; i < images.size(); i++) {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.setImage(images[i]).setViewType(vk::ImageViewType::e2D).setFormat(format);
        viewInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);

        depackReturnValue(views[i], device.createImageView(viewInfo));
    }
}

} // namespace TBE::Graphics
