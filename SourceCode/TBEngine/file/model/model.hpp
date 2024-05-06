#pragma once

#include "TBEngine/file/base/fileBase.hpp"
#include "TBEngine/math/dataFormat.hpp"

#include <tiny_obj_loader.h>

namespace TBE::File {

class ModelFile : public FileBase {
    using super = FileBase;

public:
    ModelFile(const std::string filePath_ = "None");

private:
    std::vector<Math::DataFormat::Vertex>  vertices{};
    std::vector<Math::DataFormat::idxType> indices{};

public:
    void                      read();
    void                      free();
    const decltype(vertices)& getVertices() { return vertices; };
    const decltype(indices)&  getIndices() { return indices; };

private:
    static std::vector<std::string> supportedShaderTypes;

private:
    bool pathValid() const override;
};


} // namespace TBE::File