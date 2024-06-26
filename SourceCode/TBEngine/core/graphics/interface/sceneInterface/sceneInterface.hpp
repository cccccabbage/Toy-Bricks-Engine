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
    void                                         initUniformBuffer();

public:
    void tickGPU(const vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout);

public:
    void                                       updateUniformBuffer(std::span<std::byte> data);
    std::span<Graphics::BufferResourceUniform> getUniformBufferRs() { return uniformBufferRs; }

private:
    std::vector<Graphics::BufferResourceUniform> uniformBufferRs{};
    uint32_t                                     currentFrame = 0;
};

} // namespace TBE::Graphics
