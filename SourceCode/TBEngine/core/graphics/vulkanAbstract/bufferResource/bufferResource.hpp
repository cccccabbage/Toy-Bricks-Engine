#pragma once

#include "TBEngine/utils/includes/includeVulkan.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/base/vulkanAbstractBase.hpp"

#include <span>
#include <tuple>

namespace TBE::Graphics {

class BufferResource : public VulkanAbstractBase {
public:
    BufferResource() : VulkanAbstractBase() {}
    ~BufferResource();
    void destroy() override;

public:
    void init(const std::span<std::byte>& inData,
              vk::BufferUsageFlags        usage,
              vk::MemoryPropertyFlags     memPro);

public:
    vk::Buffer       buffer{};
    vk::DeviceMemory memory{};
    vk::DeviceSize   size{};

protected:
    [[nodiscard]] std::tuple<vk::Buffer, vk::DeviceMemory>
    createBuffer(vk::DeviceSize                     size,
                 vk::BufferUsageFlags               usage,
                 vk::MemoryPropertyFlags            memPro,
                 vk::PhysicalDeviceMemoryProperties phyMemPro);
};

class BufferResourceUniform : public BufferResource {
public:
    BufferResourceUniform() {}
    ~BufferResourceUniform();
    void destroy() override;

public:
    void init(vk::DeviceSize size, const vk::PhysicalDeviceMemoryProperties& phyMemPro);

    void update(const std::span<std::byte>& newData);

public:
    void* mapPtr = nullptr;

protected:
    size_t bufferSize{};

private:
    void init(const vk::Device*                         pDevice_,
              const std::span<std::byte>&               inData,
              vk::BufferUsageFlags                      usage,
              vk::MemoryPropertyFlags                   memPro,
              const vk::PhysicalDeviceMemoryProperties& phyMemPro) = delete;
};

} // namespace TBE::Graphics
