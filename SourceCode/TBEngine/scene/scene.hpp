#pragma once

#include "TBEngine/utils/includes/includeVulkan.hpp"
#include "TBEngine/resource/shader/shader.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"
#include "model/model.hpp"
#include "camera/camera.hpp"
#include "TBEngine/enums.hpp"

#include <vector>
#include <string_view>
#include <tuple>
#include <any>

namespace TBE::Scene {

class Scene {
public:
    void destroy();

public:
    std::vector<std::tuple<InputType, std::any>> getBindFuncs();

public:
    void tickCPU();
    void tickGPU(const vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout);

public: // model related
    // call addModel(...) for all the models needed to read before call read();
    void read();
    void addModel(std::string_view modelPath, std::string_view texturePath);

public: // shader related
    void addShader(std::string filePath, ShaderType type) { shader.addShader(filePath, type); }

    const std::vector<vk::PipelineShaderStageCreateInfo> initDescriptorSetLayout();
    const vk::DescriptorSetLayout&                       getDescriptorSetLayout();
    void                                                 destroyShaderCache();
    void initDescriptorPool(uint32_t maxSets, const std::span<vk::DescriptorPoolSize> poolSizes);
    void initDescriptorSets(const std::span<Graphics::BufferResourceUniform> uniBuffers,
                            const vk::Sampler&                               sampler,
                            const vk::ImageView&                             sampleTarget);

public:
    const vk::Sampler&   getTextureSampler(uint32_t idx);
    const vk::ImageView& getTextureImageView(uint32_t idx);
    const vk::Buffer&    getVertBuffer(uint32_t idx);
    const vk::Buffer&    getIdxBuffer(uint32_t idx);

    auto getIdxSize(uint32_t idx) { return modelManager.getIdxSize(idx); }
    std::span<Graphics::BufferResourceUniform> getUniformBufferRs() { return uniformBufferRs; }

public:
    Resource::ShaderManager shader{};

private:
    Camera                                       camera{};
    Model::ModelManager                          modelManager{};
    std::vector<Graphics::BufferResourceUniform> uniformBufferRs{};
    uint32_t                                     currentFrame = 0;

private:
    void updateUniformBuffer();
};

} // namespace TBE::Scene
