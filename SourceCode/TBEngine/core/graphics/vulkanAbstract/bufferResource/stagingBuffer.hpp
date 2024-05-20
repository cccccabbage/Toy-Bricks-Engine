#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/base/vulkanAbstractBase.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/imageResource/imageResource.hpp"

#include <functional>

namespace TBE::Graphics
{

class StagingBuffer : public VulkanAbstractBase
{
public:
    StagingBuffer() = delete;
    StagingBuffer(const std::span<std::byte>& inData);
    ~StagingBuffer();

public:
    void destroy() override;
    void copyTo(ImageResource* imageR, uint32_t width, uint32_t height);

public:
    vk::Buffer       buffer{};
    vk::DeviceMemory memory{};
    void*            data = nullptr;

private:
    void createBuffer(const std::span<std::byte>& inData);
};

} // namespace TBE::Graphics
