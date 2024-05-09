#include "stagingBuffer.hpp"
#include "TBEngine/graphics/detail/graphicsDetail.hpp"

namespace TBE::Graphics {

using namespace TBE::Graphics::Detail;

StagingBuffer::StagingBuffer(const vk::Device&           device_,
                             const vk::PhysicalDevice&   phyDevice_,
                             const std::span<std::byte>& inData)
    : VulkanAbstractBase(device_, phyDevice_) {
    createBuffer(inData);
}

StagingBuffer::~StagingBuffer() { destroy(); }

void StagingBuffer::destroy() {
    device.destroy(buffer);
    device.free(memory);
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

void StagingBuffer::createBuffer(const std::span<std::byte>& inData) {
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.setSize(inData.size())
        .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
        .setSharingMode(vk::SharingMode::eExclusive);
    depackReturnValue(buffer, device.createBuffer(bufferInfo));

    auto memReq = device.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memReq.size)
        .setMemoryTypeIndex(findMemoryType(phyDevice.getMemoryProperties(),
                                           memReq.memoryTypeBits,
                                           vk::MemoryPropertyFlagBits::eHostVisible |
                                               vk::MemoryPropertyFlagBits::eHostCoherent));
    depackReturnValue(memory, device.allocateMemory(allocInfo));
    handleVkResult(device.bindBufferMemory(buffer, memory, 0));

    depackReturnValue(data, device.mapMemory(memory, 0, inData.size()));
    std::memcpy(data, inData.data(), static_cast<size_t>(inData.size()));
    device.unmapMemory(memory);
}

} // namespace TBE::Graphics
