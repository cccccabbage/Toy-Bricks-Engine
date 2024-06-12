#pragma once

#include "TBEngine/resource/file/texture/textureFile.hpp"

#include <filesystem>


namespace TBE::Resource {} // namespace TBE::Resource

namespace TBE::Resource {

class Texture {
public:
    Texture();

    ~Texture();
    void destroy();

    void init(const std::filesystem::path& filePath_, bool slowRead = false);

public:
    // read in the file and init imageR and sampler, release the texture data itself as long as it
    // has been read into staging buffer
    void read();

public:
    File::TextureFile file{};
};

} // namespace TBE::Resource
