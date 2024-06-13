#include "model.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/core/graphics/graphics.hpp"


namespace TBE::Scene::Model {

size_t ModelManager::add(std::string_view modelPath, std::string_view texturePath, bool slowRead) {
    auto& modelFile = modelFiles.emplace_back();
    modelFile.newFile(modelPath);
    if (!modelFile.isValid()) {
        Utils::Log::logErrorMsg("invalid file path for model");
    }

    auto& textureFile = textureFiles.emplace_back();
    textureFile.newFile(texturePath);
    if (!textureFile.isValid()) {
        Utils::Log::logErrorMsg("invalid file path for texture");
    }

    auto idx = modelFiles.size() - 1;
    if (!slowRead) {
        read(idx);
    }

    return idx;
}

void ModelManager::destroy() {
    textureFiles.clear();
    modelFiles.clear();
}

void ModelManager::read(size_t idx) {
    auto& modelFile   = modelFiles[idx];
    auto& textureFile = textureFiles[idx];

    modelFile.read();
    Graphics::VulkanGraphics::modelInterface.read(
        modelFile.getVerticesByte(), modelFile.getIndicesByte(), modelFile.getIndices().size());
    Graphics::VulkanGraphics::textureInterface.read(textureFile.read());
}

} // namespace TBE::Scene::Model
