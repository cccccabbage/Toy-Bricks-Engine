#include "stagingBuffer.hpp"
#include "TBEngine/graphics/detail/graphicsDetail.hpp"

namespace TBE::Graphics {

using namespace TBE::Graphics::Detail;

StagingBuffer::StagingBuffer(const vk::Device*                  pDevice_,
                             const std::span<std::byte>&        inData,
                             vk::PhysicalDeviceMemoryProperties phyMemPro) {
    setPDevice(pDevice_);
    createBuffer(inData, phyMemPro);
}

StagingBuffer::~StagingBuffer() { destroy(); }

void StagingBuffer::destroy() {
    if (!pDevice) return;
    pDevice->destroy(buffer);
    pDevice->free(memory);
    pDevice = nullptr;
}

void StagingBuffer::copyTo(
    ImageResource*                                               imageR,
    uint32_t                                                     width,
    uint32_t                                                     height,
    std::function<void(std::function<void(vk::CommandBuffer&)>)> disposableCommands) {
    vk::BufferImageCopy region{};
    region.setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(0)
        .setImageOffset({0, 0, 0})
        .setImageExtent({width, height, 1});
    region.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setMipLevel(0)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    disposableCommands([this, &imageR, &region](vk::CommandBuffer& cmdBuffer) {
        cmdBuffer.copyBufferToImage(
            this->buffer, imageR->image, vk::ImageLayout::eTransferDstOptimal, region);
    });
}

void StagingBuffer::createBuffer(const std::span<std::byte>&        inData,
                                 vk::PhysicalDeviceMemoryProperties phyMemPro) {
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.setSize(inData.size())
        .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
        .setSharingMode(vk::SharingMode::eExclusive);
    depackReturnValue(buffer, pDevice->createBuffer(bufferInfo));

    auto memReq = pDevice->getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memReq.size)
        .setMemoryTypeIndex(findMemoryType(phyMemPro,
                                           memReq.memoryTypeBits,
                                           vk::MemoryPropertyFlagBits::eHostVisible |
                                               vk::MemoryPropertyFlagBits::eHostCoherent));
    depackReturnValue(memory, pDevice->allocateMemory(allocInfo));
    handleVkResult(pDevice->bindBufferMemory(buffer, memory, 0));

    depackReturnValue(data, pDevice->mapMemory(memory, 0, inData.size()));
    std::memcpy(data, inData.data(), static_cast<size_t>(inData.size()));
    pDevice->unmapMemory(memory);
}

} // namespace TBE::Graphics