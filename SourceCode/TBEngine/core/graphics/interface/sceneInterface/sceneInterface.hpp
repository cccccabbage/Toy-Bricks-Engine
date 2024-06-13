#pragma once
#include "TBEngine/utils/includes/includeVulkan.hpp"
#include "TBEngine/enums.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"

#include <any>

namespace TBE::Graphics {

class SceneInterface {
public:
    void destroy();

public:
    std::vector<std::tuple<InputType, std::any>> getBindFuncs();

public:
    void tickGPU(const vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout);

public: // shader related
    const std::vector<vk::PipelineShaderStageCreateInfo> initDescriptorSetLayout();
    const vk::DescriptorSetLayout&                       getDescriptorSetLayout();
    void                                                 destroyShaderCache();
    void initDescriptorPool(uint32_t maxSets, const std::span<vk::DescriptorPoolSize> poolSizes);
    void initDescriptorSets(const std::span<Graphics::BufferResourceUniform> uniBuffers,
                            const vk::Sampler&                               sampler,
                            const vk::ImageView&                             sampleTarget);
    void initUniformBuffer();

public:
    const vk::Sampler&   getTextureSampler(uint32_t idx);
    const vk::ImageView& getTextureImageView(uint32_t idx);
    const vk::Buffer&    getVertBuffer(uint32_t idx);
    const vk::Buffer&    getIdxBuffer(uint32_t idx);
    const size_t         getIdxSize(uint32_t idx);

    std::span<Graphics::BufferResourceUniform> getUniformBufferRs() { return uniformBufferRs; }

    std::vector<Graphics::BufferResourceUniform> uniformBufferRs{};
    uint32_t                                     currentFrame = 0;

public:
    void updateUniformBuffer(std::span<std::byte> data);
};

} // namespace TBE::Graphics
