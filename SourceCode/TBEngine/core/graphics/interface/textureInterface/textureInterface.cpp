#include "textureInterface.hpp"
#include "TBEngine/core/graphics/graphics.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/bufferResource/stagingBuffer.hpp"


namespace TBE::Graphics {
using namespace TBE::Utils::Log;
using TBE::Graphics::StagingBuffer;

TextureInterface::TextureInterface() : super(), imageR() {
}

TextureInterface::~TextureInterface() {
    destroy();
}

void TextureInterface::destroy() {
    static bool destroyed = false;
    if (!destroyed) {
        device.destroy(sampler);
        imageR.destroy();
        destroyed = true;
    }
}

void TextureInterface::read(Resource::File::TextureContent* pTexContent) {
    auto           pixels    = pTexContent->pixels;
    int            texHeight = pTexContent->texHeight, texWidth = pTexContent->texWidth;
    vk::DeviceSize imageSize = texHeight * texWidth * 4;
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    std::span<std::byte> data(static_cast<std::byte*>(static_cast<void*>(pixels)), imageSize);
    StagingBuffer        stagingBuffer{data};
    pTexContent->free();

    imageR.setFormat(vk::Format::eR8G8B8A8Srgb);
    imageR.setWH(texWidth, texHeight);
    imageR.init(ImageResourceType::eTexture);

    transitionImageLayout({.image     = imageR.image,
                           .format    = vk::Format::eR8G8B8A8Srgb,
                           .oldLayout = vk::ImageLayout::eUndefined,
                           .newLayout = vk::ImageLayout::eTransferDstOptimal,
                           .mipLevels = mipLevels});
    stagingBuffer.copyTo(
        &imageR, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    generateMipmaps();

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.setMagFilter(vk::Filter::eLinear)
        .setMinFilter(vk::Filter::eLinear)
        .setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setAnisotropyEnable(vk::True)
        .setMaxAnisotropy(phyDevice.getProperties().limits.maxSamplerAnisotropy)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
        .setUnnormalizedCoordinates(vk::False)
        .setCompareEnable(vk::False)
        .setCompareOp(vk::CompareOp::eAlways)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setMaxLod(static_cast<float>(mipLevels));

    depackReturnValue(sampler, device.createSampler(samplerInfo));
}

void TextureInterface::transitionImageLayout(__TextureTransitionImageLayoutArgs args) {
    auto& [image, format, oldLayout, newLayout, mipLevels] = args;
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange.setBaseMipLevel(0).setLevelCount(mipLevels).setBaseArrayLayer(0).setLayerCount(
        1);

    vk::ImageMemoryBarrier barrier{};
    barrier.setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setImage(image)
        .setSubresourceRange(subresourceRange);

    vk::PipelineStageFlags sourceStage{};
    vk::PipelineStageFlags destinationStage{};
    if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
            .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

        sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        sourceStage      = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        logErrorMsg("unsupported layout transition!");
    }

    auto func = [&sourceStage, &destinationStage, &barrier](vk::CommandBuffer& cmdBuffer) {
        cmdBuffer.pipelineBarrier( // TODO: what's this
            sourceStage,
            destinationStage,
            {},
            {},
            {},
            barrier);
    };

    Graphics::disposableCommands(func);
}

void TextureInterface::generateMipmaps() {
    auto formatProperties = phyDevice.getFormatProperties(imageR.format);
    if (!(formatProperties.optimalTilingFeatures &
          vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        logErrorMsg("texture image format does not support linear blitting!");
    }

    Graphics::disposableCommands([this](const vk::CommandBuffer& cmdBuffer) {
        vk::ImageMemoryBarrier barrier{};
        barrier.image                           = imageR.image;
        barrier.srcQueueFamilyIndex             = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex             = vk::QueueFamilyIgnored;
        barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        barrier.subresourceRange.levelCount     = 1;

        int32_t mipWidth = imageR.width, mipHeight = imageR.height;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout                     = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask                 = vk::AccessFlagBits::eTransferRead;

            cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                      vk::PipelineStageFlagBits::eTransfer,
                                      {},
                                      {},
                                      {},
                                      barrier);

            vk::ImageBlit blit{};
            blit.setSrcOffsets({{{0, 0, 0}, {mipWidth, mipHeight, 1}}})
                .setDstOffsets(
                    {{{0, 0, 0},
                      {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}}});
            blit.srcSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
            blit.srcSubresource.mipLevel       = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount     = 1;

            blit.dstSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
            blit.dstSubresource.mipLevel       = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount     = 1;

            cmdBuffer.blitImage(imageR.image,
                                vk::ImageLayout::eTransferSrcOptimal,
                                imageR.image,
                                vk::ImageLayout::eTransferDstOptimal,
                                blit,
                                vk::Filter::eLinear);

            barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

            cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                      vk::PipelineStageFlagBits::eFragmentShader,
                                      {},
                                      {},
                                      {},
                                      barrier);
            if (mipWidth > 1)
                mipWidth /= 2;
            if (mipHeight > 1)
                mipHeight /= 2;
        }
        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                  vk::PipelineStageFlagBits::eFragmentShader,
                                  {},
                                  {},
                                  {},
                                  barrier);
    });
}

} // namespace TBE::Graphics
