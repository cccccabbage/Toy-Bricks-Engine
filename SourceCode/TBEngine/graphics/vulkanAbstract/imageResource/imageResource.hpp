#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/base/base.hpp"

namespace TBE::Graphics {

class ImageResource : public VulkanAbstractBase {
public:
    ImageResource() = default;
    ~ImageResource();

public:
    void initAll(const vk::Device*                         pDevice_,
                 const vk::ImageCreateInfo&                imageInfo,
                 vk::ImageViewCreateInfo&                  viewInfo,
                 const vk::PhysicalDeviceMemoryProperties& memPro,
                 const vk::MemoryPropertyFlags&            reqPro);

    void createImage(const vk::ImageCreateInfo& imageInfo);
    void createBuffer(const vk::PhysicalDeviceMemoryProperties& memPro,
                      const vk::MemoryPropertyFlags&            reqPro);
    void createImageView(vk::ImageViewCreateInfo& viewInfo);

    void destroy() override;

public:
    vk::Image        image{};
    vk::ImageView    imageView{};
    vk::DeviceMemory memory{};
};

} // namespace TBE::Graphics