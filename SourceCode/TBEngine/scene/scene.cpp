#include "scene.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/core/graphics/graphics.hpp"


namespace TBE::Scene
{

void Scene::destroy()
{
    std::for_each(models.begin(), models.end(), [](Model::Model& model) { model.destroy(); });
    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [](Graphics::BufferResourceUniform& buffer) { buffer.destroy(); });
}

void Scene::tick()
{
    updateUniformBuffer();
    currentImage = (currentImage + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Scene::updateUniformBuffer()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    const auto& extent    = Graphics::VulkanGraphics::extent;

    auto  currentTime = std::chrono::high_resolution_clock::now();
    float time =
        std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    Math::DataFormat::UniformBufferObject ubo{};
    ubo.model =
        glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj =
        glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1; // important: Vulkan has a different coordinates from OpenGL

    std::span<std::byte> data(reinterpret_cast<std::byte*>(&ubo), sizeof(ubo));
    uniformBufferRs[currentImage].update(data);
}

void Scene::read()
{
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

void Scene::addModel(const __SceneAddModelArgs args)
{
    models.emplace_back();
    models.back().init(
        {.modelPath = args.modelPath, .texturePath = args.texturePath, .slowRead = true});
}

} // namespace TBE::Scene
