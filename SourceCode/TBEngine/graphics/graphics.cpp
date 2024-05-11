#include "TBEngine/graphics/graphics.hpp"

#include "TBEngine/graphics/detail/graphicsDetail.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/window/window.hpp"
#include "TBEngine/math/dataFormat.hpp"
#include "TBEngine/file/model/model.hpp"
#include "TBEngine/ui/ui.hpp"

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

VulkanGraphics::VulkanGraphics(Window::Window& window_) : window(window_) { initVulkan(); }

VulkanGraphics::~VulkanGraphics() { cleanup(); }

void VulkanGraphics::initVulkan() {
    logger->trace("Initializing graphic.");

    createInstance();
    createSurface();

    setupDebugMessenger();

    pickPhysicalDevice();
    createExtent();
    createLogicalDevice();

    createSwapChain();

    createRenderPass();
    createGraphicsPipeline();

    createColorResources();
    createDepthResources();
    createFramebuffers();
    createCommandPool();

    createTexture();

    loadModel();
    createUniformBuffers();
    createDescriptor();

    createCommandBuffers();
    createSyncObjects();

    logger->trace("Graphic initialized.");
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

    updateUniformBuffer(currentFrame);

    device.resetFences(fence);

    cmdBuffer.reset();

    recordCommandBuffer(cmdBuffer, imageIndex);

    vk::Semaphore          waitSemaphores[]   = {imgAviSemaphore};
    vk::PipelineStageFlags waitStages[]       = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore          signalSemaphores[] = {renFinSemaphore};
    vk::SubmitInfo         submitInfo{};
    submitInfo.setWaitSemaphoreCount(1)
        .setPWaitSemaphores(waitSemaphores)
        .setPWaitDstStageMask(waitStages)
        .setCommandBufferCount(1)
        .setPCommandBuffers(&cmdBuffer)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(signalSemaphores);

    handleVkResult(graphicsQueue.submit(submitInfo, fence));

    vk::SubpassDependency dependency{};
    dependency.setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::SwapchainKHR   swapchains[] = {swapchainR.swapchain};
    vk::PresentInfoKHR presentInfo{};
    presentInfo.setWaitSemaphoreCount(1)
        .setPWaitSemaphores(signalSemaphores)
        .setSwapchainCount(1)
        .setPSwapchains(swapchains)
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

    texture.destroy();

    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [](BufferResourceUniform& uniBuf) { uniBuf.destroy(); });

    shader.destroy();

    indexBufferRC.destroy();
    vertexBufferRC.destroy();

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
    if (inDebug) { instance.destroy(debugMessenger, nullptr); }
    instance.destroy(surface);
    instance.destroy();
}

bool* VulkanGraphics::getPFrameBufferResized() { return &framebufferResized; }

void VulkanGraphics::createInstance() {
    if (inDebug && !checkValidationLayerSupport(validationLayers)) {
        logErrorMsg("Validation layers requested, but not available.");
    }

    vk::ApplicationInfo appInfo{};
    appInfo.setPApplicationName("Toy Bricks Engine")
        .setApplicationVersion(1)
        .setPEngineName("No Engine")
        .setEngineVersion(1)
        .setApiVersion(VK_API_VERSION_1_0);

    auto extensions = getRequiredExtensions();

    vk::InstanceCreateInfo createInfo{};
    createInfo.setPApplicationInfo(&appInfo)
        .setEnabledLayerCount(0)
        .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
        .setPpEnabledExtensionNames(extensions.data());

    auto dbgCreateInfo = genDebugCreateInfo();
    if (inDebug) {
        createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()))
            .setPpEnabledLayerNames(validationLayers.data())
            .setPNext(&dbgCreateInfo);
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
    if (!inDebug) return;

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
    if (devices.size() == 0) { logErrorMsg("Failed to find GPUs with Vulkan support."); }

    for (const auto& phyDeivce_ : devices) {
        if (isDeviceSuitable(phyDeivce_)) {
            phyDevice   = phyDeivce_;
            msaaSamples = getMaxUsableSampleCount(phyDevice);
            break;
        }
    }

    if (!phyDevice) { logErrorMsg("Failed to find suitable GPU."); }
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
        .setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
        .setPQueueCreateInfos(queueCreateInfos.data())
        .setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
        .setPpEnabledExtensionNames(deviceExtensions.data())
        .setPEnabledFeatures(&deviceFeatures);

    depackReturnValue(device, phyDevice.createDevice(createInfo));

    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue  = device.getQueue(indices.presentFamily.value(), 0);
}

void VulkanGraphics::createSwapChain() { swapchainR.init(phyDevice, window.getFramebufferSize()); }

void VulkanGraphics::createRenderPass() {
    renderPass.init(swapchainR.format, findDepthFormat(), msaaSamples);
}

void VulkanGraphics::createGraphicsPipeline() {
    shader.addShader("Shaders/vert.spv", ShaderType::eVertex);
    shader.addShader("Shaders/frag.spv", ShaderType::eFrag);
    auto shaderStages = shader.doneShaderAdding(); // descriptor set layout inited here

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                   vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
        .setPDynamicStates(dynamicStates.data());

    auto bindingDescription    = getBindingDescription();
    auto attributeDescriptions = getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.setVertexBindingDescriptionCount(1)
        .setPVertexBindingDescriptions(&bindingDescription)
        .setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
        .setPVertexAttributeDescriptions(attributeDescriptions.data());

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
    viewportState.setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(
        &scissor);

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
    colorBlending.setLogicOpEnable(vk::False).setAttachmentCount(1).setPAttachments(
        &colorBlendAttachment);

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLess) // lower depth means closer
        .setDepthBoundsTestEnable(vk::False)
        .setStencilTestEnable(vk::False);

    // define the uniform data that would be passed to shader
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setSetLayoutCount(1).setPSetLayouts(&shader.descriptors.layout);

    depackReturnValue(pipelineLayout, device.createPipelineLayout(pipelineLayoutInfo));

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.setStageCount(static_cast<uint32_t>(shaderStages.size()))
        .setPStages(shaderStages.data())
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

    shader.destroyCache(); // shaderModules, createInfos and descriptor set layout bindings
}

void VulkanGraphics::createFramebuffers() {
    swapChainFramebuffers.resize(swapchainR.views.size());

    for (size_t i = 0; i < swapchainR.views.size(); i++) {
        std::array attachments = {
            colorImageR.imageView, depthImageR.imageView, swapchainR.views[i]};
        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.setRenderPass(renderPass.renderPass)
            .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
            .setPAttachments(attachments.data())
            .setWidth(extent.width)
            .setHeight(extent.height)
            .setLayers(1);

        depackReturnValue(swapChainFramebuffers[i], device.createFramebuffer(framebufferInfo));
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

void VulkanGraphics::createTexture() {
    texture.init(
        "Resources/Textures/viking_room.png",
        [this](std::function<void(vk::CommandBuffer&)> func) { disposableCommands(func); });
}

void VulkanGraphics::loadModel() {
    model.read();
    auto           vertices   = model.getVertices();
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    std::span<std::byte> verData(reinterpret_cast<std::byte*>(vertices.data()), bufferSize);
    vertexBufferRC.init(
        verData,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        [this](std::function<void(vk::CommandBuffer&)> func) { disposableCommands(func); });

    auto indices = model.getIndices();
    bufferSize   = sizeof(indices[0]) * indices.size();

    std::span<std::byte> idxData(reinterpret_cast<std::byte*>(indices.data()), bufferSize);
    indexBufferRC.init(
        idxData,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        [this](std::function<void(vk::CommandBuffer&)> func) { disposableCommands(func); });

    // model.free() should not be called as long as there is a need to draw the model
}

void VulkanGraphics::createUniformBuffers() {
    vk::DeviceSize bufferSize = sizeof(Math::DataFormat::UniformBufferObject);

    uniformBufferRs.resize(MAX_FRAMES_IN_FLIGHT, {device, phyDevice});
    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [this, bufferSize](BufferResourceUniform& unibuf) {
                      unibuf.init(bufferSize, phyDevice.getMemoryProperties());
                  });
}

void VulkanGraphics::createDescriptor() {
    std::array<vk::DescriptorPoolSize, 2> poolSizes{};
    poolSizes[0]
        .setType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    poolSizes[1]
        .setType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

    shader.descriptors.initPool(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), poolSizes);

    shader.descriptors.initSets(uniformBufferRs, texture.sampler, texture.imageR.imageView);
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

    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        device.destroy(swapChainFramebuffers[i]);
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

void VulkanGraphics::updateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto  currentTime = std::chrono::high_resolution_clock::now();
    float time =
        std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    Math::DataFormat::UniformBufferObject ubo{};
    ubo.model =
        glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj =
        glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1; // important: Vulkan has a different coordinates from OpenGL

    std::span<std::byte> data(reinterpret_cast<std::byte*>(&ubo), sizeof(ubo));
    uniformBufferRs[currentImage].update(data);
}

vk::ShaderModule VulkanGraphics::createShaderModule(const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.setCodeSize(code.size()).setPCode(reinterpret_cast<const uint32_t*>(code.data()));

    vk::ShaderModule shaderModule{};
    depackReturnValue(shaderModule, device.createShaderModule(createInfo));

    return std::move(shaderModule);
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

void VulkanGraphics::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo{};
    handleVkResult(commandBuffer.begin(beginInfo));

    vk::Rect2D renderArea{};
    renderArea.setOffset({0, 0}).setExtent(extent);
    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].setColor({0.0f, 0.0f, 0.0f, 1.0f});
    clearValues[1].setDepthStencil({1.0f, 0});

    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.setRenderPass(renderPass.renderPass)
        .setFramebuffer(swapChainFramebuffers[imageIndex])
        .setRenderArea(renderArea)
        .setClearValueCount(static_cast<uint32_t>(clearValues.size()))
        .setPClearValues(clearValues.data());
    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    vk::Viewport viewport{};
    viewport.setX(0.0f)
        .setY(0.0f)
        .setWidth(static_cast<float>(extent.width))
        .setHeight(static_cast<float>(extent.height))
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);
    commandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor{};
    scissor.setOffset({0, 0}).setExtent(extent);
    commandBuffer.setScissor(0, scissor);

    std::array                                       vertexBuffers = {vertexBufferRC.buffer};
    std::array<vk::DeviceSize, vertexBuffers.size()> offsets       = {0};
    commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
    commandBuffer.bindIndexBuffer(indexBufferRC.buffer, 0, vk::IndexType::eUint32);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     pipelineLayout,
                                     0,
                                     shader.descriptors.sets[currentFrame],
                                     static_cast<uint32_t>(0));

    commandBuffer.drawIndexed(static_cast<uint32_t>(model.getIndices().size()), 1, 0, 0, 0);

    commandBuffer.endRenderPass();
    handleVkResult(commandBuffer.end());
}

vk::Format VulkanGraphics::findDepthFormat() {
    return findSupportedFormat(
        phyDevice,
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

void VulkanGraphics::disposableCommands(std::function<void(vk::CommandBuffer&)> func) {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(commandPool)
        .setCommandBufferCount(1);

    std::vector<vk::CommandBuffer> cmdBuffers{};
    depackReturnValue(cmdBuffers, device.allocateCommandBuffers(allocInfo));
    auto cmdBuffer = cmdBuffers[0];

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    handleVkResult(cmdBuffer.begin(beginInfo));

    func(cmdBuffer);

    handleVkResult(cmdBuffer.end());

    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBufferCount(1).setPCommandBuffers(&cmdBuffer);

    handleVkResult(graphicsQueue.submit(submitInfo));
    while (graphicsQueue.waitIdle() == vk::Result::eTimeout) {
        logger->warn("graphicsQueue waitIdle is disposableCommands(): timeout.");
    }

    device.free(commandPool, 1, &cmdBuffer);
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
