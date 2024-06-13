#include "sceneInterface.hpp"
#include "TBEngine/core/graphics/graphics.hpp"


namespace TBE::Graphics {

void SceneInterface::destroy() {
    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [](Graphics::BufferResourceUniform& buffer) { buffer.destroy(); });
}

void SceneInterface::tickGPU(const vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout) {
    std::array                                       vertexBuffers = {getVertBuffer(0)};
    std::array<vk::DeviceSize, vertexBuffers.size()> offsets       = {0};
    cmdBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
    cmdBuffer.bindIndexBuffer(getIdxBuffer(0), 0, vk::IndexType::eUint32);
    cmdBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        layout,
        0,
        Graphics::VulkanGraphics::shaderInterface.descriptors.sets[currentFrame],
        static_cast<uint32_t>(0));
    cmdBuffer.drawIndexed(static_cast<uint32_t>(getIdxSize(0)), 1, 0, 0, 0);
}

void SceneInterface::updateUniformBuffer(std::span<std::byte> data) {
    uniformBufferRs[currentFrame].update(data);
}

void SceneInterface::initUniformBuffer() {
    vk::DeviceSize bufferSize = sizeof(Math::DataFormat::UniformBufferObject);
    uniformBufferRs.resize(MAX_FRAMES_IN_FLIGHT);
    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [bufferSize](Graphics::BufferResourceUniform& buffer) {
                      buffer.init(bufferSize,
                                  Graphics::VulkanGraphics::phyDevice.getMemoryProperties());
                  });
}

const std::vector<vk::PipelineShaderStageCreateInfo> SceneInterface::initDescriptorSetLayout() {
    return Graphics::VulkanGraphics::shaderInterface.initDescriptorSetLayout();
}

const vk::DescriptorSetLayout& SceneInterface::getDescriptorSetLayout() {
    return Graphics::VulkanGraphics::shaderInterface.descriptors.layout;
}

void SceneInterface::destroyShaderCache() {
    Graphics::VulkanGraphics::shaderInterface.destroyCache();
}

void SceneInterface::initDescriptorPool(uint32_t                                maxSets,
                                        const std::span<vk::DescriptorPoolSize> poolSizes) {
    Graphics::VulkanGraphics::shaderInterface.descriptors.initPool(maxSets, poolSizes);
}

void SceneInterface::initDescriptorSets(const std::span<Graphics::BufferResourceUniform> uniBuffers,
                                        const vk::Sampler&                               sampler,
                                        const vk::ImageView& sampleTarget) {
    Graphics::VulkanGraphics::shaderInterface.descriptors.initSets(
        uniBuffers, sampler, sampleTarget);
}

const vk::Sampler& SceneInterface::getTextureSampler(uint32_t idx) {
    return Graphics::VulkanGraphics::modelInterface.getTextureSampler(idx);
}

const vk::ImageView& SceneInterface::getTextureImageView(uint32_t idx) {
    return Graphics::VulkanGraphics::modelInterface.getTextureImageView(idx);
}

const vk::Buffer& SceneInterface::getVertBuffer(uint32_t idx) {
    return Graphics::VulkanGraphics::modelInterface.getVertBuffer(idx);
}

const vk::Buffer& SceneInterface::getIdxBuffer(uint32_t idx) {
    return Graphics::VulkanGraphics::modelInterface.getIdxBuffer(idx);
}

const size_t SceneInterface ::getIdxSize(uint32_t idx) {
    return Graphics::VulkanGraphics::modelInterface.getIdxSize(idx);
}

} // namespace TBE::Graphics
