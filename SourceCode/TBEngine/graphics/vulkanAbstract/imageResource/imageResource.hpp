#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/base/vulkanAbstractBase.hpp"

namespace TBE::Graphics {

enum class ImageResourceType {
    eUnknown = 0,
    eColor,
    eDepth,
    eTexture,
};

class ImageResource : public VulkanAbstractBase {
public:
    ImageResource() = delete;
    ImageResource(const vk::Device& device_, const vk::PhysicalDevice& phyDevice_)
        : VulkanAbstractBase(device_, phyDevice_) {}
    ~ImageResource();

public:
    void init(ImageResourceType imgType);

    void setFormat(vk::Format format_);
    void setWH(uint32_t width_, uint32_t height_);
    void createImage(const vk::ImageCreateInfo& imageInfo);
    void createBuffer(const vk::PhysicalDeviceMemoryProperties& memPro,
                      const vk::MemoryPropertyFlags&            reqPro);
    void createImageView(vk::ImageViewCreateInfo& viewInfo);

    void destroy() override;

public:
    void init(const vk::ImageCreateInfo&                imageInfo,
              vk::ImageViewCreateInfo&                  viewInfo,
              const vk::PhysicalDeviceMemoryProperties& memPro,
              const vk::MemoryPropertyFlags&            reqPro);

public:
    vk::Image        image{};
    vk::ImageView    imageView{};
    vk::DeviceMemory memory{};

    uint32_t   width{0};
    uint32_t   height{0};
    vk::Format format{vk::Format::eR8G8B8A8Srgb};

private:
    std::tuple<vk::ImageCreateInfo, vk::ImageViewCreateInfo>
    createImageInfos(uint32_t                mipLevels_,
                     vk::SampleCountFlagBits numSamples,
                     vk::Format              format,
                     vk::ImageTiling         tiling,
                     vk::ImageUsageFlags     usage,
                     vk::ImageAspectFlags    aspectFlags);

private:
    bool imageInited  = false;
    bool viewInited   = false;
    bool memoryInited = false;
};

} // namespace TBE::Graphics
