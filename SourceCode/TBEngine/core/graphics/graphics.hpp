#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"

#include "TBEngine/core/graphics/vulkanAbstract/imageResource/imageResource.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/swapchainResource/swapchainResource.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/renderPass/renderPass.hpp"
#include "TBEngine/scene/scene.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <functional>

namespace TBE::Window {
class Window;
}

namespace TBE {
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
}

namespace TBE::Graphics {
const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> deviceExtensions = {vk::KHRSwapchainExtensionName};

void disposableCommands(std::function<void(vk::CommandBuffer&)> func);

class VulkanGraphics final {
private:
    friend void disposableCommands(std::function<void(vk::CommandBuffer&)> func);

public:
    VulkanGraphics(Window::Window& window_);
    ~VulkanGraphics();

public:
    void tick();

private:
    void initVulkan();
    void cleanup();

public:
    bool*                     getPFrameBufferResized();
    void                      bindTickCmdFunc(std::function<void(const vk::CommandBuffer&)> func);
    ImGui_ImplVulkan_InitInfo getImguiInfo();

    auto getBindFuncs() { return scene.getBindFuncs(); }

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
    void loadModel();
    void createDescriptor();
    void createCommandBuffers();
    void createSyncObjects();

private:
    void cleanupSwapChain();
    void recreateSwapChain();

private:
    vk::Queue                      presentQueue{};
    SwapchainResource              swapchainR{};
    RenderPass                     renderPass{};
    std::vector<vk::Framebuffer>   swapchainFramebuffers{};
    ImageResource                  depthImageR{};
    ImageResource                  colorImageR{};
    std::vector<vk::CommandBuffer> commandBuffers{};
    std::vector<vk::Semaphore>     imageAvailableSemaphores{};
    std::vector<vk::Semaphore>     renderFinishedSemaphores{};
    std::vector<vk::Fence>         inFlightFences{};
    vk::PipelineLayout             pipelineLayout{};
    vk::Pipeline                   graphicsPipeline{};
    uint32_t                       mipLevels{};
    vk::SampleCountFlagBits        msaaSamples = vk::SampleCountFlagBits::e1;
    vk::DebugUtilsMessengerEXT     debugMessenger{};

private:
    std::vector<std::function<void(const vk::CommandBuffer&)>> tickCmdFuncs{};

private:
    Scene::Scene scene{};

private:
    bool       isDeviceSuitable(const vk::PhysicalDevice& phyDevice);
    void       recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    vk::Format findDepthFormat();

private:
    Window::Window& window;

    uint32_t currentFrame       = 0;
    bool     framebufferResized = false;

public:
    static vk::Instance       instance;
    static vk::PhysicalDevice phyDevice;
    static vk::Device         device;
    static vk::SurfaceKHR     surface;
    static vk::Extent2D       extent;

private:
    static vk::CommandPool commandPool;
    static vk::Queue       graphicsQueue;
};

} // namespace TBE::Graphics
