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
}

} // namespace TBE::Resource
