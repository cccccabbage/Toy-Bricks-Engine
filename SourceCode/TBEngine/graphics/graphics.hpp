#pragma once

#include "TBEngine/file/model/model.hpp"
#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/imageResource/imageResource.hpp"
#include "TBEngine/graphics/vulkanAbstract/swapchainResource/swapchainResource.hpp"
#include "TBEngine/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"
#include "TBEngine/graphics/vulkanAbstract/renderPass/renderPass.hpp"
#include "TBEngine/graphics/vulkanAbstract/descriptor/descriptor.hpp"

#include <functional>


namespace TBE::Window {
class Window;
}

namespace TBE::Graphics {

class VulkanGraphics {
public:
    VulkanGraphics(const Window::Window* window);

public:
    void initVulkan();
    void tick();
    void cleanup();

public:
    bool* getPFrameBufferResized();

private:
    void createInstance();
    void createSurface();
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createExtent();
    void createLogicalDevice();
    void createSwapChain();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createColorResources();
    void createDepthResources();
    void createTextureImage();
    void createTextureSampler();
    void loadModel();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();

private:
    void cleanupSwapChain();
    void recreateSwapChain();
    void updateUniformBuffer(uint32_t currentImage);

private:
    vk::Instance                   instance{};
    vk::PhysicalDevice             physicalDevice{};
    vk::Device                     device{};
    vk::Queue                      graphicsQueue{};
    vk::Queue                      presentQueue{};
    vk::SurfaceKHR                 surface{};
    vk::Extent2D                   extent{};
    vk::PipelineLayout             pipelineLayout{};
    vk::Pipeline                   graphicsPipeline{};
    vk::CommandPool                commandPool{};
    std::vector<vk::CommandBuffer> commandBuffers{};
    std::vector<vk::Semaphore>     imageAvailableSemaphores{};
    std::vector<vk::Semaphore>     renderFinishedSemaphores{};
    std::vector<vk::Fence>         inFlightFences{};
    RenderPass                     renderPass{};
    Descriptor                     descriptor{};

    SwapchainResource swapchainR{};

    std::vector<vk::Framebuffer> swapChainFramebuffers{};

    uint32_t mipLevels{};

    ImageResource textureImageR{};
    vk::Sampler   textureSampler{};

    BufferResource                     vertexBufferRC{};
    BufferResource                     indexBufferRC{};
    std::vector<BufferResourceUniform> uniformBufferRs;

    ImageResource depthImageR{};
    ImageResource colorImageR{};

    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> deviceExtensions = {vk::KHRSwapchainExtensionName};
    vk::DebugUtilsMessengerEXT     debugMessenger   = {};

private:
    TBE::File::ModelFile model{"Resources/Models/viking_room.obj"};

private:
    vk::ShaderModule createShaderModule(const std::vector<char>& code);
    bool             isDeviceSuitable(const vk::PhysicalDevice& device);
    void             recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    vk::Format       findSupportedFormat(const std::vector<vk::Format>& candidates,
                                         vk::ImageTiling                tiling,
                                         vk::FormatFeatureFlags         features);
    vk::Format       findDepthFormat();

    [[nodiscard]] std::tuple<vk::Buffer, vk::DeviceMemory>
    createBuffer(vk::DeviceSize          size,
                 vk::BufferUsageFlags    usage,
                 vk::MemoryPropertyFlags properties);

    std::tuple<vk::ImageCreateInfo,
               vk::ImageViewCreateInfo,
               vk::PhysicalDeviceMemoryProperties,
               vk::MemoryPropertyFlags>
    createImageInfos(uint32_t                width,
                     uint32_t                height,
                     uint32_t                mipLevels_,
                     vk::SampleCountFlagBits numSamples,
                     vk::Format              format,
                     vk::ImageTiling         tiling,
                     vk::ImageUsageFlags     usage,
                     vk::MemoryPropertyFlags properties,
                     vk::ImageAspectFlags    aspectFlags);

    void disposableCommands(std::function<void(vk::CommandBuffer&)> func);

    void transitionImageLayout(vk::Image       image,
                               vk::Format      format,
                               vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               uint32_t        mipLevels_);
    void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

    void generateMipmaps(vk::Image  image,
                         vk::Format imageFormat,
                         int32_t    texWidth,
                         int32_t    texHeight,
                         uint32_t   mipLevels_);

    vk::SampleCountFlagBits getMaxUsableSampleCount();

private:
    const Window::Window* window = nullptr;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t  currentFrame         = 0;
    bool      framebufferResized   = false;

private:
    vk::PhysicalDeviceMemoryProperties phyMemPro{};
};

} // namespace TBE::Graphics
