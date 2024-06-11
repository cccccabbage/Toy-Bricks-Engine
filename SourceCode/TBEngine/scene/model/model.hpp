#pragma once

#include "TBEngine/resource/file/model/modelFile.hpp"
#include "TBEngine/resource/texture/texture.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"

#include <string_view>
#include <vector>

namespace TBE::Scene::Model {

// manager for all the model and texture files
// maintain a table for the map between models and textures
class ModelManager {
public:
    size_t init(std::string_view modelPath, std::string_view texturePath, bool slowRead);
    void   destroy();

public:
    void   read(size_t idx);
    bool   empty() { return modelFiles.empty(); }
    size_t size() { return modelFiles.size(); }

public:
    const auto& getVertBuffer(uint32_t idx) { return vertBufs[idx].buffer; }
    const auto& getIdxBuffer(uint32_t idx) { return idxBufs[idx].buffer; }
    const auto& getTextureSampler(uint32_t idx) { return textureFiles[idx].sampler; }
    const auto& getTextureImageView(uint32_t idx) { return textureFiles[idx].imageR.imageView; }

    const auto getIdxSize(uint32_t idx) { return modelFiles[idx].getIndices().size(); }

private:
    std::vector<Resource::File::ModelFile> modelFiles{};
    std::vector<Resource::Texture>         textureFiles{};

    // TODO: remove these two
    std::vector<Graphics::BufferResource> vertBufs{};
    std::vector<Graphics::BufferResource> idxBufs{};
};

} // namespace TBE::Scene::Model
