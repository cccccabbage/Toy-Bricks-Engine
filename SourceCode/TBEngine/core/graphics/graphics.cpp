#include "graphics.hpp"

#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/core/graphics/detail/graphicsDetail.hpp"
#include "TBEngine/core/window/window.hpp"
#include "TBEngine/settings.hpp"

#include <utility>


PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateDebugUtilsMessengerEXT(VkInstance                                instance,
                               const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                               const VkAllocationCallbacks*              pAllocator,
                               VkDebugUtilsMessengerEXT*                 pMessenger);

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                                           VkDebugUtilsMessengerEXT     messenger,
                                                           VkAllocationCallbacks const* pAllocator);

namespace TBE::Graphics { // VulkanGraphics
using namespace TBE::Graphics::Detail;
vk::Instance       VulkanGraphics::instance         = {};
vk::PhysicalDevice VulkanGraphics::phyDevice        = {};
vk::Device         VulkanGraphics::device           = {};
vk::SurfaceKHR     VulkanGraphics::surface          = {};
vk::CommandPool    VulkanGraphics::commandPool      = {};
vk::Queue          VulkanGraphics::graphicsQueue    = {};
vk::Extent2D       VulkanGraphics::extent           = {{WINDOW_WIDTH, WINDOW_HEIGHT}};
ShaderInterface    VulkanGraphics::shaderInterface  = {};
TextureInterface   VulkanGraphics::textureInterface = {};
ModelInterface     VulkanGraphics::modelInterface   = {};
SceneInterface     VulkanGraphics::sceneInterface   = {};


VulkanGraphics::VulkanGraphics(Window::Window& window_) : window(window_) {
    logger->trace("Initializing graphic.");
    initVulkan();

    logger->trace("Graphic initialized.");
}

VulkanGraphics::~VulkanGraphics() {
    cleanup();
}

void VulkanGraphics::initVulkan() {
    createInstance();
    createSurface();

    setupDebugMessenger();

    pickPhysicalDevice();
    createExtent();
    createLogicalDevice();

    createSwapChain();

    createRenderPass();

    createColorResources();
    createDepthResources();
    createFramebuffers();
    createCommandPool();

    createCommandBuffers();
    createSyncObjects();
}

void VulkanGraphics::tick() {
    vk::Fence&         fence           = inFlightFences[currentFrame];
    vk::Semaphore&     imgAviSemaphore = imageAvailableSemaphores[currentFrame];
    vk::Semaphore&     renFinSemaphore = renderFinishedSemaphores[currentFrame];
    vk::CommandBuffer& cmdBuffer       = commandBuffers[currentFrame];

    while (device.waitForFences(fence, vk::True, std::numeric_limits<uint64_t>::max()) ==
           vk::Result::eTimeout) {
        logger->warn("wait for fences: timeout.");
    }

    auto [result, imageIndex] = device.acquireNextImageKHR(
        swapchainR.swapchain, std::numeric_limits<uint64_t>::max(), imgAviSemaphore, nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return;
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        logErrorMsg("failed to acquire swap chain image!");
    }

    device.resetFences(fence);

    cmdBuffer.reset();

    recordCommandBuffer(cmdBuffer, imageIndex);

    std::array             waitSemaphores   = {imgAviSemaphore};
    vk::PipelineStageFlags waitStages[]     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    std::array             signalSemaphores = {renFinSemaphore};
    vk::SubmitInfo         submitInfo{};

    submitInfo.setWaitSemaphores(waitSemaphores)
        .setPWaitDstStageMask(waitStages)
        .setCommandBuffers(cmdBuffer)
        .setSignalSemaphores(signalSemaphores);

    handleVkResult(graphicsQueue.submit(submitInfo, fence));

    vk::SubpassDependency dependency{};
    dependency.setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::PresentInfoKHR presentInfo{};
    presentInfo.setWaitSemaphores(signalSemaphores)
        .setSwapchains(swapchainR.swapchain)
        .setPImageIndices(&imageIndex);

    result = presentQueue.presentKHR(presentInfo);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR ||
        framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != vk::Result::eSuccess) {
        logErrorMsg("failed to present!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanGraphics::cleanup() {
    while (device.waitIdle() == vk::Result::eTimeout) {
        logger->warn("device waitIdle in cleanup(): timeout.");
    }

    cleanupSwapChain();

    shaderInterface.destroy();
    textureInterface.destroy();
    modelInterface.destroy();
    sceneInterface.destroy();

    device.destroy(graphicsPipeline);
    device.destroy(pipelineLayout);
    renderPass.destroy();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        device.destroy(imageAvailableSemaphores[i]);
        device.destroy(renderFinishedSemaphores[i]);
        device.destroy(inFlightFences[i]);
    }

    device.destroy(commandPool, nullptr); // command buffers are freed implicitly here.

    device.destroy(); // This would implicitly clean up the device queue.
    if (inDebug) {
        instance.destroy(debugMessenger, nullptr);
    }
    instance.destroy(surface);
    instance.destroy();
}

bool* VulkanGraphics::getPFrameBufferResized() {
    return &framebufferResized;
}

void VulkanGraphics::bindTickCmdFunc(std::function<void(const vk::CommandBuffer&)> func) {
    tickCmdFuncs.emplace_back(func);
}

void VulkanGraphics::initSceneInterface() {
    createGraphicsPipeline();
    createDescriptor();

    auto sceneTickFunc =
        std::bind(&SceneInterface::tickGPU, &sceneInterface, std::placeholders::_1, pipelineLayout);
    bindTickCmdFunc(sceneTickFunc);
}

ImGui_ImplVulkan_InitInfo VulkanGraphics::getImguiInfo() {
    ImGui_ImplVulkan_InitInfo info{};
    info.Queue          = graphicsQueue;
    info.Device         = device;
    info.PhysicalDevice = phyDevice;
    info.Subpass        = 0;
    info.Instance       = instance;
    info.Allocator      = nullptr;
    info.ImageCount     = MAX_FRAMES_IN_FLIGHT;
    info.RenderPass     = renderPass.renderPass;
    info.MSAASamples    = (VkSampleCountFlagBits)msaaSamples;
    info.QueueFamily    = QueueFamilyIndices(phyDevice, surface).graphicsFamily.value();
    info.MinImageCount  = MAX_FRAMES_IN_FLIGHT;
    return info;
}

void VulkanGraphics::createInstance() {
    if (inDebug && !checkValidationLayerSupport(validationLayers)) {
        logErrorMsg("Validation layers requested, but not available.");
    };
    vk::ApplicationInfo appInfo{};
    appInfo.setPApplicationName("Toy Bricks Engine")
        .setApplicationVersion(1)
        .setPEngineName("No Engine")
        .setEngineVersion(1)
        .setApiVersion(VK_API_VERSION_1_0);

    auto extensions = getRequiredExtensions();

    vk::InstanceCreateInfo createInfo{};
    createInfo.setPApplicationInfo(&appInfo).setEnabledLayerCount(0).setPEnabledExtensionNames(
        extensions);

    auto dbgCreateInfo = genDebugCreateInfo();
    if (inDebug) {
        createInfo.setPEnabledLayerNames(validationLayers).setPNext(&dbgCreateInfo);
    }

    depackReturnValue(instance, vk::createInstance(createInfo));
}

void VulkanGraphics::createSurface() {
    auto createInfo = vk::Win32SurfaceCreateInfoKHR()
                          .setHwnd(window.getWin32Window())
                          .setHinstance(window.getModuleHandle());
    depackReturnValue(surface, instance.createWin32SurfaceKHR(createInfo));
}

void VulkanGraphics::setupDebugMessenger() {
    if (!inDebug)
        return;

    auto createInfo = genDebugCreateInfo();

    pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
    pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
    if (!pfnVkCreateDebugUtilsMessengerEXT || !pfnVkDestroyDebugUtilsMessengerEXT) {
        logErrorMsg("GetInstanceProcAddr: Unable to find pfnCreateDebugUtilsMessengerEXT or "
                    "pfnVkDestroyDebugUtilsMessengerEXT function.");
    }

    depackReturnValue(debugMessenger, instance.createDebugUtilsMessengerEXT(createInfo));
}

void VulkanGraphics::pickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> devices{};
    depackReturnValue(devices, instance.enumeratePhysicalDevices());
    if (devices.size() == 0) {
        logErrorMsg("Failed to find GPUs with Vulkan support.");
    }

    for (const auto& phyDeivce_ : devices) {
        if (isDeviceSuitable(phyDeivce_)) {
            phyDevice   = phyDeivce_;
            msaaSamples = getMaxUsableSampleCount(phyDevice);
            break;
        }
    }

    if (!phyDevice) {
        logErrorMsg("Failed to find suitable GPU.");
    }
}

void VulkanGraphics::createExtent() {
    vk::SurfaceCapabilitiesKHR capabilities{};
    depackReturnValue(capabilities, phyDevice.getSurfaceCapabilitiesKHR(surface));
    extent = chooseSwapExtent(capabilities, window.getFramebufferSize());
}

void VulkanGraphics::createLogicalDevice() {
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

    QueueFamilyIndices indices             = QueueFamilyIndices(phyDevice, surface);
    std::set<uint32_t> uniqueQueueFamilies = indices;

    float queuePriority = 1.0f;
    for (auto queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.setQueueFamilyIndex(queueFamily)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }
    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.setSamplerAnisotropy(vk::True);

    vk::DeviceCreateInfo createInfo{};
    createInfo.setFlags(vk::DeviceCreateFlags())
        .setQueueCreateInfos(queueCreateInfos)
        .setPEnabledExtensionNames(deviceExtensions)
        .setPEnabledFeatures(&deviceFeatures);

    depackReturnValue(device, phyDevice.createDevice(createInfo));

    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue  = device.getQueue(indices.presentFamily.value(), 0);
}

void VulkanGraphics::createSwapChain() {
    swapchainR.init(phyDevice, window.getFramebufferSize());
}

void VulkanGraphics::createRenderPass() {
    renderPass.init(swapchainR.format, findDepthFormat(), msaaSamples);
}

void VulkanGraphics::createGraphicsPipeline() {
    // shader loading is managed by TBEngine, in engine.cpp
    auto shaderStages = shaderInterface.initDescriptorSetLayout();

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                   vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.setDynamicStates(dynamicStates);

    auto bindingDescription    = getBindingDescription();
    auto attributeDescriptions = getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.setVertexBindingDescriptions(bindingDescription)
        .setVertexAttributeDescriptions(attributeDescriptions);

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(vk::False);

    vk::Viewport viewport{
        0.0f,                 // x
        0.0f,                 // y
        (float)extent.width,  // width
        (float)extent.height, // height
        0.0f,                 // minDepth
        1.0f                  // maxDepth
        // minDepth can be larger than maxDepth
    };

    vk::Rect2D scissor{
        {0, 0}, // offset
        extent  // swapChainExtent
    };

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.setViewports(viewport).setScissors(scissor);

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1.0f)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eCounterClockwise)
        .setDepthBiasEnable(vk::False);

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.setSampleShadingEnable(vk::True)
        .setMinSampleShading(0.2f)
        .setRasterizationSamples(msaaSamples);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
        .setBlendEnable(vk::True)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
        .setAlphaBlendOp(vk::BlendOp::eAdd);

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.setLogicOpEnable(vk::False).setAttachments(colorBlendAttachment);

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLess) // lower depth means closer
        .setDepthBoundsTestEnable(vk::False)
        .setStencilTestEnable(vk::False);

    // define the uniform data that would be passed to shader
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setSetLayouts(shaderInterface.descriptors.layout);

    depackReturnValue(pipelineLayout, device.createPipelineLayout(pipelineLayoutInfo));

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.setStages(shaderStages)
        .setPVertexInputState(&vertexInputInfo)
        .setPInputAssemblyState(&inputAssembly)
        .setPViewportState(&viewportState)
        .setPRasterizationState(&rasterizer)
        .setPMultisampleState(&multisampling)
        .setPColorBlendState(&colorBlending)
        .setPDynamicState(&dynamicState)
        .setLayout(pipelineLayout)
        .setRenderPass(renderPass.renderPass)
        .setSubpass(0)
        .setPDepthStencilState(&depthStencil);

    std::vector<vk::Pipeline> grapPipes{};
    depackReturnValue(grapPipes, device.createGraphicsPipelines(nullptr, pipelineInfo));
    graphicsPipeline = std::move(grapPipes[0]);

    shaderInterface.destroyCache();
}

void VulkanGraphics::createFramebuffers() {
    swapchainFramebuffers.resize(swapchainR.views.size());

    for (size_t i = 0; i < swapchainR.views.size(); i++) {
        std::array attachments = {
            colorImageR.imageView, depthImageR.imageView, swapchainR.views[i]};
        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.setRenderPass(renderPass.renderPass)
            .setAttachments(attachments)
            .setWidth(extent.width)
            .setHeight(extent.height)
            .setLayers(1);

        depackReturnValue(swapchainFramebuffers[i], device.createFramebuffer(framebufferInfo));
    }
}

void VulkanGraphics::createCommandPool() {
    auto indices = QueueFamilyIndices(phyDevice, surface);

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(indices.graphicsFamily.value());

    depackReturnValue(commandPool, device.createCommandPool(poolInfo));
}

void VulkanGraphics::createColorResources() {
    colorImageR.setFormat(swapchainR.format);
    colorImageR.setWH(extent.width, extent.height);
    colorImageR.init(ImageResourceType::eColor);
}

void VulkanGraphics::createDepthResources() {
    depthImageR.setFormat(findDepthFormat());
    depthImageR.setWH(extent.width, extent.height);
    depthImageR.init(ImageResourceType::eDepth);
}

void VulkanGraphics::createDescriptor() {
    std::array<vk::DescriptorPoolSize, 2> poolSizes{};
    poolSizes[0]
        .setType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    poolSizes[1]
        .setType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

    shaderInterface.descriptors.initPool(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), poolSizes);
    shaderInterface.descriptors.initSets(sceneInterface.getUniformBufferRs(),
                                         modelInterface.getTextureSampler(0),
                                         modelInterface.getTextureImageView(0));
}

void VulkanGraphics::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setCommandPool(commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));

    depackReturnValue(commandBuffers, device.allocateCommandBuffers(allocInfo));
}

void VulkanGraphics::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo     fenceInfo{};
    fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        depackReturnValue(imageAvailableSemaphores[i], device.createSemaphore(semaphoreInfo));
        depackReturnValue(renderFinishedSemaphores[i], device.createSemaphore(semaphoreInfo));
        depackReturnValue(inFlightFences[i], device.createFence(fenceInfo));
    }
}

void VulkanGraphics::cleanupSwapChain() {
    colorImageR.destroy();
    depthImageR.destroy();

    for (size_t i = 0; i < swapchainFramebuffers.size(); i++) {
        device.destroy(swapchainFramebuffers[i]);
    }

    swapchainR.destroy();
}

void VulkanGraphics::recreateSwapChain() {
    auto bufferSize = window.getFramebufferSize();
    while (bufferSize.width == 0 || bufferSize.height == 0) {
        bufferSize = window.getFramebufferSize();
        window.waitEvents();
    }

    while (device.waitIdle() == vk::Result::eTimeout) {
        logger->warn("device waitIdle in recreateSwapChain(): timeout.");
    }

    cleanupSwapChain();

    createSwapChain();
    createExtent();
    createColorResources();
    createDepthResources();
    createFramebuffers();
}

bool VulkanGraphics::isDeviceSuitable(const vk::PhysicalDevice& phyDevice) {
    QueueFamilyIndices indices = QueueFamilyIndices(phyDevice, surface);

    auto extensionsSupported = checkDeviceExtensionSupport(phyDevice, deviceExtensions);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport{phyDevice, surface};
        swapChainAdequate =
            !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    auto supportedFeatures = phyDevice.getFeatures();

    return indices.isComplete() && extensionsSupported && swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;
}

void VulkanGraphics::recordCommandBuffer(vk::CommandBuffer cmdBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo{};
    handleVkResult(cmdBuffer.begin(beginInfo));

    vk::Rect2D renderArea{};
    renderArea.setOffset({0, 0}).setExtent(extent);
    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].setColor({0.0f, 0.0f, 0.0f, 1.0f});
    clearValues[1].setDepthStencil({1.0f, 0});

    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.setRenderPass(renderPass.renderPass)
        .setFramebuffer(swapchainFramebuffers[imageIndex])
        .setRenderArea(renderArea)
        .setClearValues(clearValues);
    cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    vk::Viewport viewport{};
    viewport.setX(0.0f)
        .setY(0.0f)
        .setWidth(static_cast<float>(extent.width))
        .setHeight(static_cast<float>(extent.height))
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);
    cmdBuffer.setViewport(0, viewport);

    vk::Rect2D scissor{};
    scissor.setOffset({0, 0}).setExtent(extent);
    cmdBuffer.setScissor(0, scissor);

    for (auto& func : tickCmdFuncs) {
        func(cmdBuffer);
    }

    cmdBuffer.endRenderPass();
    handleVkResult(cmdBuffer.end());
}

vk::Format VulkanGraphics::findDepthFormat() {
    return findSupportedFormat(
        phyDevice,
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

void disposableCommands(std::function<void(vk::CommandBuffer&)> func) {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(VulkanGraphics::commandPool)
        .setCommandBufferCount(1);

    std::vector<vk::CommandBuffer> cmdBuffers{};
    depackReturnValue(cmdBuffers, VulkanGraphics::device.allocateCommandBuffers(allocInfo));
    auto cmdBuffer = cmdBuffers[0];

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    handleVkResult(cmdBuffer.begin(beginInfo));

    func(cmdBuffer);

    handleVkResult(cmdBuffer.end());

    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBuffers(cmdBuffer);

    handleVkResult(VulkanGraphics::graphicsQueue.submit(submitInfo));
    while (VulkanGraphics::graphicsQueue.waitIdle() == vk::Result::eTimeout) {
        logger->warn("graphicsQueue waitIdle in disposableCommands(): timeout.");
    }

    VulkanGraphics::device.free(VulkanGraphics::commandPool, 1, &cmdBuffer);
}

} // namespace TBE::Graphics


VKAPI_ATTR VkResult VKAPI_CALL
vkCreateDebugUtilsMessengerEXT(VkInstance                                instance,
                               const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                               const VkAllocationCallbacks*              pAllocator,
                               VkDebugUtilsMessengerEXT*                 pMessenger) {
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL
vkDestroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                VkDebugUtilsMessengerEXT     messenger,
                                VkAllocationCallbacks const* pAllocator) {
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}
