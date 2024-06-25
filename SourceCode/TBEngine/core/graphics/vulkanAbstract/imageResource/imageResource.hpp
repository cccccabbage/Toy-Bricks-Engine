#pragma once

#include "TBEngine/utils/includes/includeVulkan.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/base/vulkanAbstractBase.hpp"
#include "TBEngine/enums.hpp"

namespace TBE::Graphics {

class ImageResource : public VulkanAbstractBase {
public:
    ImageResource() : VulkanAbstractBase() {}
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
};

} // namespace TBE::Graphics
