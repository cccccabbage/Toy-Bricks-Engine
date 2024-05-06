#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/base/base.hpp"
#include "TBEngine/graphics/vulkanAbstract/imageResource/imageResource.hpp"

#include <functional>

namespace TBE::Graphics {

class StagingBuffer : public VulkanAbstractBase {
public:
    StagingBuffer() = delete;
    StagingBuffer(const vk::Device*                  pDevice_,
                  const std::span<std::byte>&        inData,
                  vk::PhysicalDeviceMemoryProperties phyMemPro);
    ~StagingBuffer();

public:
    void destroy() override;
    void copyTo(ImageResource*                                               imageR,
                uint32_t                                                     width,
                uint32_t                                                     height,
                std::function<void(std::function<void(vk::CommandBuffer&)>)> disposableCommands);

public:
    vk::Buffer       buffer{};
    vk::DeviceMemory memory{};
    void*            data = nullptr;

private:
    void createBuffer(const std::span<std::byte>&        inData,
                      vk::PhysicalDeviceMemoryProperties phyMemPro);
};

} // namespace TBE::Graphics