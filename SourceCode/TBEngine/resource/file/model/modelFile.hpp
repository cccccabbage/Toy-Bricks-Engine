#pragma once

#include "TBEngine/resource/file/base/fileBase.hpp"
#include "TBEngine/core/math/dataFormat.hpp"

#include <tiny_obj_loader.h>
#include <vector>
#include <span>

namespace TBE::Resource::File
{

class ModelFile : public FileBase
{
    using super = FileBase;

public:
    ModelFile(const std::string filePath_ = "None");

private:
    std::vector<Math::DataFormat::Vertex>  vertices{};
    std::vector<Math::DataFormat::idxType> indices{};

public:
    void                       read();
    void                       free();
    const decltype(vertices)&  getVertices() { return vertices; };
    const decltype(indices)&   getIndices() { return indices; };
    const std::span<std::byte> getVerticesByte()
    {
        return std::span<std::byte>(reinterpret_cast<std::byte*>(vertices.data()),
                                    vertices.size() * sizeof(vertices[0]));
    }
    const std::span<std::byte> getIndicesByte()
    {
        return std::span<std::byte>(reinterpret_cast<std::byte*>(indices.data()),
                                    indices.size() * sizeof(indices[0]));
    }

private:
    static std::vector<std::string> supportedShaderTypes;

private:
    bool checkPathValid() override;
    void releaseOldFile() override;
    void prepareNewFile() override;
};

} // namespace TBE::Resource::File
