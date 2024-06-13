#include "modelInterface.hpp"
#include "TBEngine/core/graphics/graphics.hpp"

namespace TBE::Graphics {

void ModelInterface::destroy() {
    vertBufs.clear();
    idxBufs.clear();
}

void ModelInterface::read(const std::span<std::byte> vertices,
                          const std::span<std::byte> indices,
                          const size_t               idxSize) {
    idxSizes.emplace_back(idxSize);
    auto& vertBuf = vertBufs.emplace_back();
    auto& idxBuf  = idxBufs.emplace_back();
    vertBuf.init(vertices,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal);

    idxBuf.init(indices,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                vk::MemoryPropertyFlagBits::eDeviceLocal);
}

const vk::Buffer& ModelInterface::getVertBuffer(uint32_t idx) {
    return vertBufs[idx].buffer;
}

const vk::Buffer& ModelInterface::getIdxBuffer(uint32_t idx) {
    return idxBufs[idx].buffer;
}

const vk::Sampler& ModelInterface::getTextureSampler(uint32_t idx) {
    return Graphics::VulkanGraphics::textureInterface.sampler;
}

const vk::ImageView& ModelInterface::getTextureImageView(uint32_t idx) {
    return Graphics::VulkanGraphics::textureInterface.imageR.imageView;
}

const size_t ModelInterface::getIdxSize(uint32_t idx) {
    return idxSizes[idx];
}


} // namespace TBE::Graphics
