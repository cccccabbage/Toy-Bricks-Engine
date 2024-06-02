#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/resource/shader/shader.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"
#include "model/model.hpp"
#include "camera/camera.hpp"

#include <vector>
#include <string_view>
#include <tuple>
#include <any>

namespace TBE::Scene {

struct __SceneAddModelArgs {
    std::string_view modelPath   = nullptr;
    std::string_view texturePath = nullptr;
};

} // namespace TBE::Scene

namespace TBE::Scene {

class Scene {
public:
    void destroy();

public:
    std::vector<std::tuple<TBE::Editor::DelegateManager::InputType, std::any>> getBindFuncs();

public:
    void tickCPU();
    void tickGPU(const vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout);

public: // model related
    // call addModel(...) for all the models needed to read before call read();
    void read();
    void addModel(const __SceneAddModelArgs args = {});

public: // shader related
    void addShader(std::string filePath, Resource::ShaderType type)
    {
        shader.addShader(filePath, type);
    }

    auto        initDescriptorSetLayout() { return shader.initDescriptorSetLayout(); }
    const auto& getDescriptorSetLayout() { return shader.descriptors.layout; }
    void        destroyShaderCache() { shader.destroyCache(); }
    void initDescriptorPool(uint32_t maxSets, const std::span<vk::DescriptorPoolSize> poolSizes)
    {
        shader.descriptors.initPool(maxSets, poolSizes);
    }
    void initDescriptorSets(const std::span<Graphics::BufferResourceUniform> uniBuffers,
                            const vk::Sampler&                               sampler,
                            const vk::ImageView&                             sampleTarget)
    {
        shader.descriptors.initSets(uniBuffers, sampler, sampleTarget);
    }

public:
    auto& getTextureSampler(int idx = -1) { return models[idx].getTextureSampler(); }
    auto& getTextureImageView(int idx = -1) { return models[idx].getTextureImageView(); }
    auto& getVertBuffer(int idx = -1) { return models[idx].getVertBuffer(); }
    auto& getIdxBuffer(int idx = -1) { return models[idx].getIdxBuffer(); }
    auto  getIdxSize(int idx = -1) { return models[idx].getIdxSize(); }

    std::span<Graphics::BufferResourceUniform> getUniformBufferRs() { return uniformBufferRs; }

public:
    Resource::Shader shader{};

private:
    Camera                                       camera {};
    std::vector<Model::Model>                    models{};
    std::vector<Graphics::BufferResourceUniform> uniformBufferRs{};
    uint32_t                                     currentFrame = 0;

private:
    void updateUniformBuffer();
};

} // namespace TBE::Scene
