#pragma once

#include "TBEngine/file/base/fileBase.hpp"

#include <stb_image.h>

#include <string>
#include <vector>

namespace TBE::File {

struct TextureContent {
    stbi_uc* pixels = nullptr;
    int      texHeight{};
    int      texWidth{};
    int      texChannel{};
};

class TextureFile : public FileBase {
    using super = FileBase;

public:
    TextureFile(const std::string& filePath_);

public:
    const TextureContent* read();
    void                  free();

private:
    TextureContent texContent{};

private:
    static std::vector<std::string> supportedTextureTypes;
    bool                            pathValid() const override;
};

} // namespace TBE::File