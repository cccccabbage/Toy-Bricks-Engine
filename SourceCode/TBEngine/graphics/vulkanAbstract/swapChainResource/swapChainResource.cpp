#include "swapchainResource.hpp"
#include "TBEngine/graphics/detail/graphicsDetail.hpp"

namespace TBE::Graphics {
using namespace TBE::Graphics::Detail;

SwapchainResource::~SwapchainResource() { destroy(); }

void SwapchainResource::destroy() {
    if (!pDevice) return;

    for (size_t i = 0; i < views.size(); i++) {
        pDevice->destroy(views[i]);
    }

    pDevice->destroy(swapchain);

    pDevice = nullptr;
}

void SwapchainResource::initAll(const vk::Device*                    pDevice_,
                                vk::SurfaceKHR*                      pSurface_,
                                const vk::PhysicalDevice&            phyDevice,
                                const std::pair<uint32_t, uint32_t>& bufferSize) {
    setPDevice(pDevice_);
    setPSurface(pSurface_);
    createSwapChain(phyDevice, bufferSize);
    createImages();
    createViews();
}

void SwapchainResource::createSwapChain(const vk::PhysicalDevice&            phyDevice,
                                        const std::pair<uint32_t, uint32_t>& bufferSize) {
    auto swapChainSupport = SwapChainSupportDetails(phyDevice, *pSurface);
    auto surfaceFormat    = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode      = chooseSwapPresentMode(swapChainSupport.presentModes);
    auto swapExtent       = chooseSwapExtent(swapChainSupport.capabilities, bufferSize);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.setSurface(*pSurface)
        .setMinImageCount(imageCount)
        .setImageFormat(surfaceFormat.format)
        .setImageColorSpace(surfaceFormat.colorSpace)
        .setImageExtent(swapExtent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    auto indices            = QueueFamilyIndices(phyDevice, pSurface);
    auto queueFamilyIndices = indices.toArray();

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndexCount(2)
            .setPQueueFamilyIndices(queueFamilyIndices.data());
    } else {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount(0)
            .setPQueueFamilyIndices(nullptr);
    }

    createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(presentMode)
        .setClipped(vk::True) // discard pixels that are obsured
        .setOldSwapchain(nullptr);

    depackReturnValue(swapchain, pDevice->createSwapchainKHR(createInfo));
    depackReturnValue(images, pDevice->getSwapchainImagesKHR(swapchain));

    format = surfaceFormat.format;
}

void SwapchainResource::createImages() {
    depackReturnValue(images, pDevice->getSwapchainImagesKHR(swapchain));
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

        depackReturnValue(views[i], pDevice->createImageView(viewInfo));
    }
}

} // namespace TBE::Graphics