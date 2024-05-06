#include "bufferResource.hpp"
#include "TBEngine/graphics/detail/graphicsDetail.hpp"
#include "stagingBuffer.hpp"

namespace TBE::Graphics {
using namespace TBE::Graphics::Detail;
using TBE::Utils::Log::logErrorMsg;

BufferResource::~BufferResource() { destroy(); }

void BufferResource::destroy() {
    if (!pDevice) return;
    pDevice->destroy(buffer);
    pDevice->free(memory);
    pDevice = nullptr;
}

void BufferResource::init(
    const vk::Device*                                            pDevice_,
    const std::span<std::byte>&                                  inData,
    vk::BufferUsageFlags                                         usage,
    vk::MemoryPropertyFlags                                      memPro,
    const vk::PhysicalDeviceMemoryProperties&                    phyMemPro,
    std::function<void(std::function<void(vk::CommandBuffer&)>)> disposableCommands) {
    pDevice = pDevice_;
    size    = inData.size();

    StagingBuffer stagingBuffer{pDevice, inData, phyMemPro};

    std::tie(buffer, memory) = createBuffer(size, usage, memPro, phyMemPro);

    disposableCommands([&stagingBuffer, this](vk::CommandBuffer& cmdBuffer) {
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

    depackReturnValue(buffer, pDevice->createBuffer(bufferInfo));

    auto memRequirements = pDevice->getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(findMemoryType(phyMemPro, memRequirements.memoryTypeBits, memPro));

    depackReturnValue(bufferMemory, pDevice->allocateMemory(allocInfo));

    handleVkResult(pDevice->bindBufferMemory(buffer, bufferMemory, 0));

    return std::make_tuple(std::move(buffer), std::move(bufferMemory));
}

BufferResourceUniform::~BufferResourceUniform() { destroy(); }

void BufferResourceUniform::destroy() {
    if (!pDevice) return;

    pDevice->destroy(buffer);
    pDevice->free(memory);
    pDevice = nullptr;
}

void BufferResourceUniform::init(const vk::Device*                         pDevice_,
                                 vk::DeviceSize                            size,
                                 const vk::PhysicalDeviceMemoryProperties& phyMemPro) {
    pDevice    = pDevice_;
    bufferSize = size;

    std::tie(buffer, memory) = createBuffer(bufferSize,
                                            vk::BufferUsageFlagBits::eUniformBuffer,
                                            vk::MemoryPropertyFlagBits::eHostVisible |
                                                vk::MemoryPropertyFlagBits::eHostCoherent,
                                            phyMemPro);
    depackReturnValue(mapPtr, pDevice->mapMemory(memory, 0, bufferSize));
}

void BufferResourceUniform::update(const std::span<std::byte>& newData) {
    if (newData.size() != bufferSize) { logErrorMsg("uniform buffer size not compatible"); }
    std::memcpy(mapPtr, newData.data(), bufferSize);
}

} // namespace TBE::Graphics
