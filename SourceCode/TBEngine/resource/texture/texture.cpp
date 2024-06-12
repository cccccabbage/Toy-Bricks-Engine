#include "texture.hpp"
#include "TBEngine/resource/file/texture/textureFile.hpp"
#include "TBEngine/core/graphics/graphics.hpp"

namespace TBE::Resource {
using namespace TBE::Utils::Log;

Texture::Texture() {
}

Texture::~Texture() {
    destroy();
}

void Texture::destroy() {
}

void Texture::init(const std::filesystem::path& filePath_, bool slowRead) {
    file.newFile(filePath_);
    if (!slowRead) {
        read();
    }
}

void Texture::read() {
    auto texContent = file.read();

    Graphics::VulkanGraphics::textureInterface.read(texContent);
}

} // namespace TBE::Resource
