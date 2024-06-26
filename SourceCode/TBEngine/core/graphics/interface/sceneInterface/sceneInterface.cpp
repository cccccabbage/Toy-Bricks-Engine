#include "sceneInterface.hpp"
#include "TBEngine/core/graphics/graphics.hpp"


namespace TBE::Graphics {

void SceneInterface::destroy() {
    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [](Graphics::BufferResourceUniform& buffer) { buffer.destroy(); });
}

void SceneInterface::tickGPU(const vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout) {
    std::array vertexBuffers = {Graphics::VulkanGraphics::modelInterface.getVertBuffer(0)};
    std::array<vk::DeviceSize, vertexBuffers.size()> offsets = {0};
    cmdBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
    cmdBuffer.bindIndexBuffer(
        Graphics::VulkanGraphics::modelInterface.getIdxBuffer(0), 0, vk::IndexType::eUint32);
    cmdBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        layout,
        0,
        Graphics::VulkanGraphics::shaderInterface.descriptors.sets[currentFrame],
        static_cast<uint32_t>(0));
    cmdBuffer.drawIndexed(
        static_cast<uint32_t>(Graphics::VulkanGraphics::modelInterface.getIdxSize(0)), 1, 0, 0, 0);
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

} // namespace TBE::Graphics
