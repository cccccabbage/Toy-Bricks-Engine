#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"
#include "model/model.hpp"

#include <vector>
#include <string_view>

namespace TBE::Scene
{

struct __SceneAddModelArgs
{
    std::string_view modelPath   = nullptr;
    std::string_view texturePath = nullptr;
};

} // namespace TBE::Scene

namespace TBE::Scene
{

class Scene
{
public:
    void destroy();

public:
    void tick();

public:
    // call addModel(...) for all the models needed to read before call read();
    void read();
    void addModel(const __SceneAddModelArgs args = {});

public:
    auto& getTextureSampler(int idx = -1) { return models[idx].getTextureSampler(); }
    auto& getTextureImageView(int idx = -1) { return models[idx].getTextureImageView(); }
    auto& getVertBuffer(int idx = -1) { return models[idx].getVertBuffer(); }
    auto& getIdxBuffer(int idx = -1) { return models[idx].getIdxBuffer(); }
    auto  getIdxSize(int idx = -1) { return models[idx].getIdxSize(); }

    std::span<Graphics::BufferResourceUniform> getUniformBufferRs() { return uniformBufferRs; }

private:
    std::vector<Model::Model>                    models{};
    std::vector<Graphics::BufferResourceUniform> uniformBufferRs{};
    uint32_t                                     currentImage = 0;

private:
    void updateUniformBuffer();
};

} // namespace TBE::Scene
