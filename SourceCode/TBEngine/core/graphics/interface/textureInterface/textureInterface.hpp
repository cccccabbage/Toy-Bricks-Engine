#pragma once

#include "TBEngine/utils/includes/includeVulkan.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/imageResource/imageResource.hpp"
#include "TBEngine/resource/file/texture/textureFile.hpp"
#include "TBEngine/core/graphics/interface/base/graphicsInterface.hpp"

#include <filesystem>

namespace TBE::Graphics {

struct __TextureTransitionImageLayoutArgs {
    vk::Image       image{};
    vk::Format      format{};
    vk::ImageLayout oldLayout{};
    vk::ImageLayout newLayout{};
    uint32_t        mipLevels{};
};

class TextureInterface final : public GraphicsInterface {
public:
    TextureInterface();

    ~TextureInterface();
    void destroy();

public:
    // read in the file and init imageR and sampler, release the texture data itself as long as it
    // has been read into staging buffer
    void read(Resource::File::TextureContent* pTexContent);

public:
    Graphics::ImageResource imageR;
    vk::Sampler             sampler{};
    uint32_t                mipLevels{0};

private:
    void transitionImageLayout(__TextureTransitionImageLayoutArgs args);
    void generateMipmaps();

private:
    using super = GraphicsInterface;
};

} // namespace TBE::Graphics
