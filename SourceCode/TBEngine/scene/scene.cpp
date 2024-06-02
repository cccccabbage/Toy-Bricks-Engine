#include "scene.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/core/graphics/graphics.hpp"


namespace TBE::Scene {
using namespace TBE::Editor::DelegateManager;

void Scene::destroy() {
    std::for_each(models.begin(), models.end(), [](Model::Model& model) { model.destroy(); });
    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [](Graphics::BufferResourceUniform& buffer) { buffer.destroy(); });

    shader.destroy();
}

std::vector<std::tuple<TBE::Editor::DelegateManager::InputType, std::any>> Scene::getBindFuncs() {
    std::any func1 = std::function<void(KeyStateMap)>(
        std::bind(&Camera::Camera::onKeyDown, &camera, std::placeholders::_1));

    return {
        std::make_tuple(InputType::eKeyBoard,  func1)
    };
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
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                 layout,
                                 0,
                                 shader.descriptors.sets[currentFrame],
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
    ubo.model =
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.view = *camera.view;
    ubo.proj = *camera.proj;

    std::span<std::byte> data(static_cast<std::byte*>(static_cast<void*>(&ubo)), sizeof(ubo));
    uniformBufferRs[currentFrame].update(data);
}

void Scene::read() {
    if (models.empty()) { logger->warn("Try to read but no model has been prepared"); }
    std::for_each(models.begin(), models.end(), [](Model::Model& model) { model.read(); });

    vk::DeviceSize bufferSize = sizeof(Math::DataFormat::UniformBufferObject);
    uniformBufferRs.resize(MAX_FRAMES_IN_FLIGHT);
    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [bufferSize](Graphics::BufferResourceUniform& buffer) {
                      buffer.init(bufferSize,
                                  Graphics::VulkanGraphics::phyDevice.getMemoryProperties());
                  });
}

void Scene::addModel(const __SceneAddModelArgs args) {
    models.emplace_back();
    models.back().init(
        {.modelPath = args.modelPath, .texturePath = args.texturePath, .slowRead = true});
}

} // namespace TBE::Scene
