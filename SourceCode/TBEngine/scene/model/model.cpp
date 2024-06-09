#include "model.hpp"
#include "TBEngine/utils/log/log.hpp"


namespace TBE::Scene::Model {

void Model::init(__ModelInitArgs args) {
    modelFile.newFile(args.modelPath);
    if (!modelFile.isValid()) {
        Utils::Log::logErrorMsg("invalid file path for model");
    }
    textureFile.init(args.texturePath, args.slowRead);
    if (!textureFile.file.isValid()) {
        Utils::Log::logErrorMsg("invalid file path for texture");
    }

    if (!args.slowRead) {
        read();
    }
}

void Model::destroy() {
    textureFile.destroy();
    modelFile.free();
    vertBuf.destroy();
    idxBuf.destroy();
}

void Model::read() {
    modelFile.read();
    textureFile.read();

    vertBuf.init(modelFile.getVerticesByte(),
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal);

    idxBuf.init(modelFile.getIndicesByte(),
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                vk::MemoryPropertyFlagBits::eDeviceLocal);
}

} // namespace TBE::Scene::Model
