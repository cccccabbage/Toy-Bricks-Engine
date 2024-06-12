#pragma once

#include "TBEngine/resource/file/base/fileBase.hpp"

#include <stb_image.h>

#include <string>
#include <vector>
#include <filesystem>

namespace TBE::Resource::File {

struct TextureContent {
    stbi_uc* pixels = nullptr;
    int      texHeight{};
    int      texWidth{};
    int      texChannel{};

    inline void free() {
        if (pixels) {
            stbi_image_free(pixels);
            *this = {};
        }
    }
};

class TextureFile : public FileBase {
    using super = FileBase;

public:
    TextureFile(const std::filesystem::path& filePath_);
    TextureFile(const std::string& filePath_);
    TextureFile(const char* filePaht_ = "");

public:
    TextureContent* read();
    void            free();

private:
    TextureContent texContent{};

private:
    static std::vector<std::string> supportedTextureTypes;
    bool                            checkPathValid() override;
    void                            releaseOldFile() override;
    void                            prepareNewFile() override;
};

} // namespace TBE::Resource::File
