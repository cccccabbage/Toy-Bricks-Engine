#pragma once

#include "TBEngine/resource/file/model/modelFile.hpp"
#include "TBEngine/resource/file/texture/textureFile.hpp"

#include <string_view>
#include <vector>

namespace TBE::Scene::Model {

// manager for all the model and texture files
// maintain a table for the map between models and textures
class ModelManager {
public:
    [[nodiscard]] size_t
         add(std::string_view modelPath, std::string_view texturePath, bool slowRead);
    void destroy();

public:
    void   read(size_t idx);
    bool   empty() { return modelFiles.empty(); }
    size_t size() { return modelFiles.size(); }

public:
    const auto getIdxSize(uint32_t idx) { return modelFiles[idx].getIndices().size(); }

private:
    std::vector<Resource::File::ModelFile>   modelFiles{};
    std::vector<Resource::File::TextureFile> textureFiles{};
};

} // namespace TBE::Scene::Model
