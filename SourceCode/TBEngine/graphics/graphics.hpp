#pragma once

#include "TBEngine/file/model/model.hpp"
#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/imageResource/imageResource.hpp"
#include "TBEngine/graphics/vulkanAbstract/swapchainResource/swapchainResource.hpp"
#include "TBEngine/graphics/vulkanAbstract/bufferResource/bufferResource.hpp"
#include "TBEngine/graphics/vulkanAbstract/renderPass/renderPass.hpp"
#include "TBEngine/graphics/texture/texture.hpp"
#include "TBEngine/graphics/shader/shader.hpp"

#include <functional>


namespace TBE::Window {
class Window;
}

namespace TBE::Graphics {

class VulkanGraphics final {
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
    void createGraphicsPipeline();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createColorResources();
    void createDepthResources();
    void createTexture();
    void loadModel();
    void createUniformBuffers();
    void createDescriptor();
    void createCommandBuffers();
    void createSyncObjects();

private:
    void cleanupSwapChain();
    void recreateSwapChain();
    void updateUniformBuffer(uint32_t currentImage);

private:
    vk::Instance                   instance{};
    vk::PhysicalDevice             phyDevice{};
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
    RenderPass                     renderPass{device, phyDevice};
    Shader                         shader{device, phyDevice};

    SwapchainResource swapchainR{device, phyDevice, surface};

    std::vector<vk::Framebuffer> swapChainFramebuffers{};

    uint32_t mipLevels{};
    Texture  texture{device, phyDevice};

    BufferResource                     vertexBufferRC{device, phyDevice};
    BufferResource                     indexBufferRC{device, phyDevice};
    std::vector<BufferResourceUniform> uniformBufferRs;

    ImageResource depthImageR{device, phyDevice};
    ImageResource colorImageR{device, phyDevice};

    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> deviceExtensions = {vk::KHRSwapchainExtensionName};
    vk::DebugUtilsMessengerEXT     debugMessenger   = {};

private:
    TBE::File::ModelFile model{"Resources/Models/viking_room.obj"};

private:
    vk::ShaderModule createShaderModule(const std::vector<char>& code);
    bool             isDeviceSuitable(const vk::PhysicalDevice& phyDevice);
    void             recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    vk::Format       findDepthFormat();

    void disposableCommands(std::function<void(vk::CommandBuffer&)> func);

private:
    const Window::Window* window = nullptr;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t  currentFrame         = 0;
    bool      framebufferResized   = false;
};

} // namespace TBE::Graphics
