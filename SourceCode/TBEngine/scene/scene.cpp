#include "scene.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/core/graphics/graphics.hpp"


namespace TBE::Scene {
using namespace TBE::Editor::DelegateManager;

void Scene::destroy() {
    modelManager.destroy();
    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [](Graphics::BufferResourceUniform& buffer) { buffer.destroy(); });

    shader.destroy();
}

std::vector<std::tuple<InputType, std::any>> Scene::getBindFuncs() {
    std::any func1 = std::function<void(KeyStateMap)>(
        std::bind(&Camera::Camera::onKeyDown, &camera, std::placeholders::_1));

    return {std::make_tuple(InputType::eKeyBoard, func1)};
}

void Scene::tickCPU() {
    camera.tickCPU();
    updateUniformBuffer();
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Scene::tickGPU(const vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout) {
    // model
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

void Scene::updateUniformBuffer() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    const auto& extent    = Graphics::VulkanGraphics::extent;

    auto  currentTime = std::chrono::high_resolution_clock::now();
    float time =
        std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    Math::DataFormat::UniformBufferObject ubo{};
    // ubo.model =
    //     glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.view = *camera.view;
    ubo.proj = *camera.proj;

    std::span<std::byte> data(static_cast<std::byte*>(static_cast<void*>(&ubo)), sizeof(ubo));
    uniformBufferRs[currentFrame].update(data);
}

void Scene::read() {
    if (modelManager.empty()) {
        logger->warn("Try to read but no model has been prepared");
    }
    for (size_t i = 0; i < modelManager.size(); i++) {
        modelManager.read(i);
    }

    vk::DeviceSize bufferSize = sizeof(Math::DataFormat::UniformBufferObject);
    uniformBufferRs.resize(MAX_FRAMES_IN_FLIGHT);
    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [bufferSize](Graphics::BufferResourceUniform& buffer) {
                      buffer.init(bufferSize,
                                  Graphics::VulkanGraphics::phyDevice.getMemoryProperties());
                  });
}

void Scene::addModel(std::string_view modelPath, std::string_view texturePath) {
    auto idx = modelManager.add(modelPath, texturePath, true);
}

const std::vector<vk::PipelineShaderStageCreateInfo> Scene::initDescriptorSetLayout() {
    return Graphics::VulkanGraphics::shaderInterface.initDescriptorSetLayout();
}

const vk::DescriptorSetLayout& Scene::getDescriptorSetLayout() {
    return Graphics::VulkanGraphics::shaderInterface.descriptors.layout;
}

void Scene::destroyShaderCache() {
    Graphics::VulkanGraphics::shaderInterface.destroyCache();
}

void Scene::initDescriptorPool(uint32_t                                maxSets,
                               const std::span<vk::DescriptorPoolSize> poolSizes) {
    Graphics::VulkanGraphics::shaderInterface.descriptors.initPool(maxSets, poolSizes);
}

void Scene::initDescriptorSets(const std::span<Graphics::BufferResourceUniform> uniBuffers,
                               const vk::Sampler&                               sampler,
                               const vk::ImageView&                             sampleTarget) {
    Graphics::VulkanGraphics::shaderInterface.descriptors.initSets(
        uniBuffers, sampler, sampleTarget);
}

const vk::Sampler& Scene::getTextureSampler(uint32_t idx) {
    return Graphics::VulkanGraphics::modelInterface.getTextureSampler(idx);
}

const vk::ImageView& Scene::getTextureImageView(uint32_t idx) {
    return Graphics::VulkanGraphics::modelInterface.getTextureImageView(idx);
}

const vk::Buffer& Scene::getVertBuffer(uint32_t idx) {
    return Graphics::VulkanGraphics::modelInterface.getVertBuffer(idx);
}

const vk::Buffer& Scene::getIdxBuffer(uint32_t idx) {
    return Graphics::VulkanGraphics::modelInterface.getIdxBuffer(idx);
}

} // namespace TBE::Scene
