#pragma once

#include "TBEngine/resource/file/model/modelFile.hpp"
#include "TBEngine/resource/texture/texture.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"

#include <string_view>

namespace TBE::Scene
{

struct __ModelInitArgs // args of Model::Init
{
    // use like this: model.init({.modelPath = "xxx", .texturePath="xxx", .slowRead=false});
    // do not create a specific object for this struct
    std::string_view modelPath   = nullptr;
    std::string_view texturePath = nullptr;
    bool             slowRead    = false;
};

} // namespace TBE::Scene

namespace TBE::Scene::Model
{

class Model
{
public:
    void init(__ModelInitArgs args = {});
    void destroy();

public:
    void read();

public:
    const auto& getVertBuffer() { return vertBuf.buffer; }
    const auto& getIdxBuffer() { return idxBuf.buffer; }
    auto        getIdxSize() { return modelFile.getIndices().size(); }
    const auto& getTextureSampler() { return textureFile.sampler; }
    const auto& getTextureImageView() { return textureFile.imageR.imageView; }


private:
    Resource::File::ModelFile modelFile{};
    Resource::Texture         textureFile{};
    Graphics::BufferResource  vertBuf{};
    Graphics::BufferResource  idxBuf{};
};

} // namespace TBE::Scene::Model
