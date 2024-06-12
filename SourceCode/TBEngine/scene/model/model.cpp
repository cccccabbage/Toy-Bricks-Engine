#include "model.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/core/graphics/graphics.hpp"


namespace TBE::Scene::Model {

size_t ModelManager::init(std::string_view modelPath, std::string_view texturePath, bool slowRead) {
    auto& modelFile = modelFiles.emplace_back();
    modelFile.newFile(modelPath);
    if (!modelFile.isValid()) {
        Utils::Log::logErrorMsg("invalid file path for model");
    }

    auto& textureFile = textureFiles.emplace_back();
    textureFile.init(texturePath, slowRead);
    if (!textureFile.file.isValid()) {
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
    // vertBuf.destroy();
    // idxBuf.destroy();
    vertBufs.clear();
    idxBufs.clear();
}

void ModelManager::read(size_t idx) {
    auto& modelFile   = modelFiles[idx];
    auto& textureFile = textureFiles[idx];
    modelFile.read();
    textureFile.read();

    auto& vertBuf = vertBufs.emplace_back();
    auto& idxBuf  = idxBufs.emplace_back();
    vertBuf.init(modelFile.getVerticesByte(),
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal);

    idxBuf.init(modelFile.getIndicesByte(),
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                vk::MemoryPropertyFlagBits::eDeviceLocal);
}

const vk::Buffer& ModelManager::getVertBuffer(uint32_t idx) {
    return vertBufs[idx].buffer;
}
const vk::Buffer& ModelManager::getIdxBuffer(uint32_t idx) {
    return idxBufs[idx].buffer;
}
const vk::Sampler& ModelManager::getTextureSampler(uint32_t idx) {
    // return textureFiles[idx].sampler;
    return Graphics::VulkanGraphics::textureInterface.sampler;
}
const vk::ImageView& ModelManager::getTextureImageView(uint32_t idx) {
    // return textureFiles[idx].imageR.imageView;
    return Graphics::VulkanGraphics::textureInterface.imageR.imageView;
}

} // namespace TBE::Scene::Model
