#include "TBEngine/graphics/vulkanAbstract/imageResource/imageResource.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/graphics/detail/graphicsDetail.hpp"

#include <string>


namespace TBE::Graphics {
using TBE::Utils::Log::logErrorMsg;
using namespace TBE::Graphics::Detail;

ImageResource::~ImageResource() { destroy(); }

void ImageResource::initAll(const vk::Device*                         pDevice_,
                            const vk::ImageCreateInfo&                imageInfo,
                            vk::ImageViewCreateInfo&                  viewInfo,
                            const vk::PhysicalDeviceMemoryProperties& memPro,
                            const vk::MemoryPropertyFlags&            reqPro) {
    if (!pDevice_) { logErrorMsg("this should not be a nullptr"); }
    setPDevice(pDevice_);
    createImage(imageInfo);
    createBuffer(memPro, reqPro);
    createImageView(viewInfo);
}

void ImageResource::createImage(const vk::ImageCreateInfo& imageInfo) {
    depackReturnValue(image, pDevice->createImage(imageInfo));
}

void ImageResource::createBuffer(const vk::PhysicalDeviceMemoryProperties& memPro,
                                 const vk::MemoryPropertyFlags&            reqPro) {
    auto                   memReq = pDevice->getImageMemoryRequirements(image);
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memReq.size)
        .setMemoryTypeIndex(findMemoryType(memPro, memReq.memoryTypeBits, reqPro));

    depackReturnValue(memory, pDevice->allocateMemory(allocInfo));
    handleVkResult(pDevice->bindImageMemory(image, memory, 0));
}

void ImageResource::createImageView(vk::ImageViewCreateInfo& viewInfo) {
    viewInfo.setImage(image);
    depackReturnValue(imageView, pDevice->createImageView(viewInfo));
}

void ImageResource::destroy() {
    if (!pDevice) return;
    pDevice->destroy(imageView);
    pDevice->destroy(image);
    pDevice->free(memory);
    pDevice = nullptr;
}

} // namespace TBE::Graphics