#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/file/texture/textureFile.hpp"
#include "TBEngine/graphics/vulkanAbstract/imageResource/imageResource.hpp"

#include <filesystem>
#include <functional>

namespace TBE::Graphics {

class Texture {
public:
    Texture() = delete;
    Texture(const vk::Device& device_, const vk::PhysicalDevice& phyDevice_);

    ~Texture();
    void destroy();

    void init(const std::filesystem::path&                                 filePath_,
              std::function<void(std::function<void(vk::CommandBuffer&)>)> disposableCommands);

public:
    std::filesystem::path filePath{};
    ImageResource         imageR;
    vk::Sampler           sampler{};
    uint32_t              mipLevels{0};

private:
    bool imageRInited  = false;
    bool samplerInited = false;

private:
    const vk::Device&         device;
    const vk::PhysicalDevice& phyDevice;

private:
    void transitionImageLayout(
        vk::Image                                                    image,
        vk::Format                                                   format,
        vk::ImageLayout                                              oldLayout,
        vk::ImageLayout                                              newLayout,
        uint32_t                                                     mipLevels_,
        std::function<void(std::function<void(vk::CommandBuffer&)>)> disposableCommands);
    void generateMipmaps(
        std::function<void(std::function<void(vk::CommandBuffer&)>)> disposableCommands);
};

} // namespace TBE::Graphics
