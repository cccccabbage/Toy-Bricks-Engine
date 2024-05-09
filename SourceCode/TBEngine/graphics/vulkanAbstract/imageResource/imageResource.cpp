#include "TBEngine/graphics/vulkanAbstract/imageResource/imageResource.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/graphics/detail/graphicsDetail.hpp"

namespace TBE::Graphics {
using TBE::Utils::Log::logErrorMsg;
using namespace TBE::Graphics::Detail;

ImageResource::~ImageResource() { destroy(); }

void ImageResource::destroy() {
    if (viewInited) {
        device.destroy(imageView);
        viewInited = false;
    }
    if (imageInited) {
        device.destroy(image);
        imageInited = false;
    }
    if (memoryInited) {
        device.free(memory);
        memoryInited = false;
    }
}

void ImageResource::init(ImageResourceType imgType) {
    if (width == 0 || height == 0) { logErrorMsg("Image width or height not inited"); }

    vk::ImageCreateInfo     imgInfo{};
    vk::ImageViewCreateInfo viewInfo{};

    uint32_t mipLevels{1};

    switch (imgType) {
        case ImageResourceType::eColor:
            std::tie(imgInfo, viewInfo) =
                createImageInfos(mipLevels,
                                 getMaxUsableSampleCount(phyDevice),
                                 format,
                                 vk::ImageTiling::eOptimal,
                                 vk::ImageUsageFlagBits::eTransientAttachment |
                                     vk::ImageUsageFlagBits::eColorAttachment,
                                 vk::ImageAspectFlagBits::eColor);
            break;
        case ImageResourceType::eDepth:
            std::tie(imgInfo, viewInfo) =
                createImageInfos(mipLevels,
                                 getMaxUsableSampleCount(phyDevice),
                                 format,
                                 vk::ImageTiling::eOptimal,
                                 vk::ImageUsageFlagBits::eDepthStencilAttachment,
                                 vk::ImageAspectFlagBits::eDepth);
            break;
        case ImageResourceType::eTexture:
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
            std::tie(imgInfo, viewInfo) = createImageInfos(mipLevels,
                                                           vk::SampleCountFlagBits::e1,
                                                           vk::Format::eR8G8B8A8Srgb,
                                                           vk::ImageTiling::eOptimal,
                                                           vk::ImageUsageFlagBits::eTransferDst |
                                                               vk::ImageUsageFlagBits::eSampled,
                                                           vk::ImageAspectFlagBits::eColor);
            break;
        case ImageResourceType::eUnknown:
        default: logErrorMsg("illeagal ImageResourceType"); break;
    }

    init(imgInfo,
         viewInfo,
         phyDevice.getMemoryProperties(),
         vk::MemoryPropertyFlagBits::eDeviceLocal);
}

void ImageResource::setFormat(vk::Format format_) { format = format_; }

void ImageResource::setWH(uint32_t width_, uint32_t height_) {
    width  = width_;
    height = height_;
}

void ImageResource::createBuffer(const vk::PhysicalDeviceMemoryProperties& memPro,
                                 const vk::MemoryPropertyFlags&            reqPro) {
    auto                   memReq = device.getImageMemoryRequirements(image);
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memReq.size)
        .setMemoryTypeIndex(findMemoryType(memPro, memReq.memoryTypeBits, reqPro));

    depackReturnValue(memory, device.allocateMemory(allocInfo));
    handleVkResult(device.bindImageMemory(image, memory, 0));
    memoryInited = true;
}

void ImageResource::createImageView(vk::ImageViewCreateInfo& viewInfo) {
    viewInfo.setImage(image);
    depackReturnValue(imageView, device.createImageView(viewInfo));
    viewInited = true;
}

void ImageResource::createImage(const vk::ImageCreateInfo& imageInfo) {
    depackReturnValue(image, device.createImage(imageInfo));
    imageInited = true;
}

void ImageResource::init(const vk::ImageCreateInfo&                imageInfo,
                         vk::ImageViewCreateInfo&                  viewInfo,
                         const vk::PhysicalDeviceMemoryProperties& memPro,
                         const vk::MemoryPropertyFlags&            reqPro) {
    setWH(imageInfo.extent.width, imageInfo.extent.height);
    createImage(imageInfo);
    createBuffer(memPro, reqPro);
    createImageView(viewInfo);
}

std::tuple<vk::ImageCreateInfo, vk::ImageViewCreateInfo>
ImageResource::createImageInfos(uint32_t                mipLevels_,
                                vk::SampleCountFlagBits numSamples,
                                vk::Format              format_,
                                vk::ImageTiling         tiling,
                                vk::ImageUsageFlags     usage,
                                vk::ImageAspectFlags    aspectFlags) {
    vk::ImageCreateInfo imageInfo{};
    imageInfo.setImageType(vk::ImageType::e2D)
        .setExtent({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1})
        .setMipLevels(mipLevels_)
        .setArrayLayers(1)
        .setFormat(format_)
        .setTiling(tiling)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setUsage(usage)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setSamples(numSamples);

    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange.setAspectMask(aspectFlags)
        .setBaseMipLevel(0)
        .setLevelCount(mipLevels_)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.setViewType(vk::ImageViewType::e2D)
        .setFormat(format_)
        .setSubresourceRange(subresourceRange);

    return std::make_tuple(imageInfo, viewInfo);
}

} // namespace TBE::Graphics
