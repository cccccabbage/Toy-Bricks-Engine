#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/base/base.hpp"

#include <span>
#include <tuple>
#include <functional>

namespace TBE::Graphics {

class BufferResource : public VulkanAbstractBase {
public:
    BufferResource() = default;
    ~BufferResource();
    void destroy() override;

public:
    void init(const vk::Device*                                            pDevice_,
              const std::span<std::byte>&                                  inData,
              vk::BufferUsageFlags                                         usage,
              vk::MemoryPropertyFlags                                      memPro,
              const vk::PhysicalDeviceMemoryProperties&                    phyMemPro,
              std::function<void(std::function<void(vk::CommandBuffer&)>)> disposableCommands);

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
    BufferResourceUniform() = default;
    ~BufferResourceUniform();
    void destroy() override;

public:
    void init(const vk::Device*                         pDevice_,
              vk::DeviceSize                            size,
              const vk::PhysicalDeviceMemoryProperties& phyMemPro);

    void update(const std::span<std::byte>& newData);

public:
    void* mapPtr = nullptr;

protected:
    size_t bufferSize{};

private:
    void
    init(const vk::Device*                                            pDevice_,
         const std::span<std::byte>&                                  inData,
         vk::BufferUsageFlags                                         usage,
         vk::MemoryPropertyFlags                                      memPro,
         const vk::PhysicalDeviceMemoryProperties&                    phyMemPro,
         std::function<void(std::function<void(vk::CommandBuffer&)>)> disposableCommands) = delete;
};

} // namespace TBE::Graphics
