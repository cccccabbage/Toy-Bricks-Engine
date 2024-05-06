#include "TBEngine/file/texture/texture.hpp"
#include "TBEngine/utils/log/log.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>
#include <filesystem>
#include <algorithm>

extern const TBE::Utils::Log::Logger* logger;

namespace TBE::File {

std::vector<std::string> TextureFile::supportedTextureTypes = {};

TextureFile::TextureFile(const std::string& filePath_) : FileBase(filePath_) {
    if (supportedTextureTypes.empty()) {
        supportedTextureTypes.emplace_back(".jpg");
        supportedTextureTypes.emplace_back(".png");
    }
    valid = pathValid();
}

const TextureContent* TextureFile::read() {
    if (texContent.pixels) return &texContent;

    int  texWidth, texHeight, texChannel;
    auto pixels =
        stbi_load(filePath.string().c_str(), &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
    if (!pixels) {
        const std::string msg = "failed to load texture image!";
        logger->error(msg);
        throw std::runtime_error(msg);
    }
    texContent = {pixels, texHeight, texWidth, texChannel};
    return &texContent;
}

void TextureFile::free() {
    stbi_image_free(texContent.pixels);
    texContent = {};
}

bool TextureFile::pathValid() const {
    bool ret = FileBase::pathValid();
    if (ret) {
        const std::string fileExt  = filePath.extension().string();
        bool              contains = false;
        auto              findExt  = [&fileExt, &contains](std::string& supExt) -> void {
            contains = contains || (supExt.compare(fileExt) == 0);
        };
        std::for_each(supportedTextureTypes.begin(), supportedTextureTypes.end(), findExt);
        ret = ret && contains;
    }
    return ret;
}

} // namespace TBE::File