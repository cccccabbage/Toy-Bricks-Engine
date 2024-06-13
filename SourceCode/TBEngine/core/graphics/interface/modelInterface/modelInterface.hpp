#pragma once
#include "TBEngine/utils/includes/includeVulkan.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"

#include <vector>

namespace TBE::Graphics {

class ModelInterface {
public:
    void destroy();

public:
    void read(const std::span<std::byte> vertices, const std::span<std::byte> indices);

public:
    const vk::Buffer&    getVertBuffer(uint32_t idx);
    const vk::Buffer&    getIdxBuffer(uint32_t idx);
    const vk::Sampler&   getTextureSampler(uint32_t idx);
    const vk::ImageView& getTextureImageView(uint32_t idx);

private:
    std::vector<Graphics::BufferResource> vertBufs{};
    std::vector<Graphics::BufferResource> idxBufs{};
};

} // namespace TBE::Graphics
