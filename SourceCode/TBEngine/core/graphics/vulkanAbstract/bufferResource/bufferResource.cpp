#include "bufferResource.hpp"
#include "TBEngine/core/graphics/detail/graphicsDetail.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/bufferResource/stagingBuffer.hpp"
#include "TBEngine/core/graphics/graphics.hpp"

namespace TBE::Graphics {
using namespace TBE::Graphics::Detail;
using TBE::Utils::Log::logErrorMsg;

BufferResource::~BufferResource() {
    destroy();
}

void BufferResource::destroy() {
    static bool destroyed = false;
    if (!destroyed) {
        device.destroy(buffer);
        device.free(memory);
        destroyed = true;
    }
}

void BufferResource::init(const std::span<std::byte>& inData,
                          vk::BufferUsageFlags        usage,
                          vk::MemoryPropertyFlags     memPro) {
    size = inData.size();

    std::tie(buffer, memory) = createBuffer(size, usage, memPro, phyDevice.getMemoryProperties());

    StagingBuffer stagingBuffer{inData};


    Graphics::disposableCommands([&stagingBuffer, this](vk::CommandBuffer& cmdBuffer) {
        vk::BufferCopy copyRegion{};
        copyRegion.setSize(this->size);
        cmdBuffer.copyBuffer(stagingBuffer.buffer, this->buffer, 1, &copyRegion);
    });
}

std::tuple<vk::Buffer, vk::DeviceMemory>
BufferResource::createBuffer(vk::DeviceSize                     size,
                             vk::BufferUsageFlags               usage,
                             vk::MemoryPropertyFlags            memPro,
                             vk::PhysicalDeviceMemoryProperties phyMemPro) {
    vk::Buffer       buffer{};
    vk::DeviceMemory bufferMemory{};

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

    depackReturnValue(buffer, device.createBuffer(bufferInfo));

    auto memRequirements = device.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(findMemoryType(phyMemPro, memRequirements.memoryTypeBits, memPro));

    depackReturnValue(bufferMemory, device.allocateMemory(allocInfo));

    handleVkResult(device.bindBufferMemory(buffer, bufferMemory, 0));

    return std::make_tuple(std::move(buffer), std::move(bufferMemory));
}

BufferResourceUniform::~BufferResourceUniform() {
    destroy();
}

void BufferResourceUniform::destroy() {
    static bool destroyed = false;
    if (!destroyed) {
        device.destroy(buffer);
        device.free(memory);
        destroyed = true;
    }
}

void BufferResourceUniform::init(vk::DeviceSize                            size,
                                 const vk::PhysicalDeviceMemoryProperties& phyMemPro) {
    bufferSize = size;

    std::tie(buffer, memory) = createBuffer(bufferSize,
                                            vk::BufferUsageFlagBits::eUniformBuffer,
                                            vk::MemoryPropertyFlagBits::eHostVisible |
                                                vk::MemoryPropertyFlagBits::eHostCoherent,
                                            phyMemPro);

    depackReturnValue(mapPtr, device.mapMemory(memory, 0, bufferSize));
}

void BufferResourceUniform::update(const std::span<std::byte>& newData) {
    if (newData.size() != bufferSize) {
        logErrorMsg("uniform buffer size not compatible");
    }
    std::memcpy(mapPtr, newData.data(), bufferSize);
}

} // namespace TBE::Graphics
