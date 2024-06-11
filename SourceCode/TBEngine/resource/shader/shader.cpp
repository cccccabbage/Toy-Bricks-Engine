#include "shader.hpp"
#include "TBEngine/core/graphics/graphics.hpp"
#include "TBEngine/core/graphics/interface/shaderInterface/shaderInterface.hpp"

namespace TBE::Resource {
using namespace TBE::Utils::Log;
using File::ShaderFile;

ShaderManager::ShaderManager() {
}

ShaderManager::~ShaderManager() {
    destroy();
}

void ShaderManager::destroy() {
    static bool destroyed = false;
    if (!destroyed) {
        destroyed = true;
    }
}

void ShaderManager::addShader(std::string filePath, ShaderType type) {
    auto& [shaderFile, _] =
        shaderFiles.emplace_back(std::move(std::make_pair(ShaderFile(filePath), type)));

    Graphics::VulkanGraphics::shaderInterface.addShader(shaderFile.read(), type);

    // ShaderFile shaderFile{filePath};
    //
    // auto shaderCode = shaderFile.read();
    // modules.emplace_back(createShaderModule(shaderCode));
    //
    // vk::PipelineShaderStageCreateInfo shaderStageInfo{{}, {}, modules.back(), "main"};
    // switch (type) {
    //     case ShaderType::eVertex:
    //         shaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
    //         break;
    //     case ShaderType::eFrag:
    //         shaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
    //         break;
    //     case ShaderType::eUnknown:
    //     default:
    //         logErrorMsg("illegal ShaderType");
    //         break;
    // }
    // stageInfos.emplace_back(shaderStageInfo);
    //
    // bindings.emplace_back(createBinding(type));
}

} // namespace TBE::Resource
