#pragma once

#include "TBEngine/resource/file/base/fileBase.hpp"
#include "TBEngine/core/math/dataFormat.hpp"

#include <tiny_obj_loader.h>
#include <vector>
#include <span>
#include <string_view>

namespace TBE::Resource::File {

class ModelFile : public FileBase {
    using super = FileBase;

public:
    ModelFile(std::string_view filePath_ = "None");

private:
    std::vector<Math::DataFormat::Vertex>  vertices{};
    std::vector<Math::DataFormat::IdxType> indices{};

public:
    void                       read();
    void                       free();
    const decltype(vertices)&  getVertices() { return vertices; };
    const decltype(indices)&   getIndices() { return indices; };
    const std::span<std::byte> getVerticesByte() {
        return std::span<std::byte>(static_cast<std::byte*>(static_cast<void*>(vertices.data())),
                                    vertices.size() * sizeof(vertices[0]));
    }
    const std::span<std::byte> getIndicesByte() {
        return std::span<std::byte>(static_cast<std::byte*>(static_cast<void*>(indices.data())),
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
