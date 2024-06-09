#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/resource/file/texture/textureFile.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/imageResource/imageResource.hpp"

#include <filesystem>


namespace TBE::Resource {
struct __TextureTransitionImageLayoutArgs {
    vk::Image       image{};
    vk::Format      format{};
    vk::ImageLayout oldLayout{};
    vk::ImageLayout newLayout{};
    uint32_t        mipLevels{};
};

} // namespace TBE::Resource

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
    File::TextureFile       file{};
    Graphics::ImageResource imageR;
    vk::Sampler             sampler{};
    uint32_t                mipLevels{0};

private:
    bool imageRInited  = false;
    bool samplerInited = false;

private:
    const vk::Device&         device;
    const vk::PhysicalDevice& phyDevice;

private:
    void transitionImageLayout(__TextureTransitionImageLayoutArgs args);
    void generateMipmaps();
};

} // namespace TBE::Resource
