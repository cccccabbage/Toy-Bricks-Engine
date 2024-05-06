#include "TBEngine/graphics/graphics.hpp"

#include "TBEngine/graphics/detail/graphicsDetail.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/window/window.hpp"
#include "TBEngine/math/dataFormat.hpp"
#include "TBEngine/file/shader/shader.hpp"
#include "TBEngine/file/texture/texture.hpp"
#include "TBEngine/file/model/model.hpp"
#include "TBEngine/graphics/vulkanAbstract/bufferResource/stagingBuffer.hpp"

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

VulkanGraphics::VulkanGraphics(const Window::Window* window_) : window(window_) {}

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
    createDescriptorSetLayout();
    createGraphicsPipeline();

    createColorResources();
    createDepthResources();
    createFramebuffers();
    createCommandPool();

    createTextureImage();
    createTextureSampler();

    loadModel();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();

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

    device.destroy(textureSampler);
    textureImageR.destroy();

    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [](BufferResourceUniform& uniBuf) { uniBuf.destroy(); });

    descriptor.destroy();

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
                          .setHwnd(window->getWin32Window())
                          .setHinstance(window->getModuleHandle());
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

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            msaaSamples    = getMaxUsableSampleCount();
            break;
        }
    }

    if (!physicalDevice) { logErrorMsg("Failed to find suitable GPU."); }
    phyMemPro = physicalDevice.getMemoryProperties();
}

void VulkanGraphics::createExtent() {
    vk::SurfaceCapabilitiesKHR capabilities{};
    depackReturnValue(capabilities, physicalDevice.getSurfaceCapabilitiesKHR(surface));
    extent = chooseSwapExtent(capabilities, window->getFramebufferSize().toPair());
}

void VulkanGraphics::createLogicalDevice() {
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

    QueueFamilyIndices indices             = QueueFamilyIndices(physicalDevice, &surface);
    std::set<uint32_t> uniqueQueueFamilies = indices.toSet();

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

    depackReturnValue(device, physicalDevice.createDevice(createInfo));

    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue  = device.getQueue(indices.presentFamily.value(), 0);
}

void VulkanGraphics::createSwapChain() {
    swapchainR.initAll(&device, &surface, physicalDevice, window->getFramebufferSize().toPair());
}

void VulkanGraphics::createRenderPass() {
    renderPass.initAll(&device, swapchainR.format, findDepthFormat(), msaaSamples);
}

void VulkanGraphics::createDescriptorSetLayout() {
    // data that would be passed to shader during render pass is defined here
    // only the data specifies as "uniform" in shader

    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex);

    vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.setBinding(1)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setPImmutableSamplers(nullptr)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    std::array bindings = {uboLayoutBinding, samplerLayoutBinding};
    descriptor.setPDevice(&device);
    descriptor.initLayout(bindings);
}

void VulkanGraphics::createGraphicsPipeline() {
    File::ShaderFile vertShader{"Shaders/vert.spv"};
    File::ShaderFile fragShader{"Shaders/frag.spv"};
    auto             vertShaderCode = vertShader.read();
    auto             fragShaderCode = fragShader.read();

    auto vertShaderModule = createShaderModule(vertShaderCode);
    auto fragShaderModule = createShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(vertShaderModule)
        .setPName("main");
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(fragShaderModule)
        .setPName("main");

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

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
    pipelineLayoutInfo.setSetLayoutCount(1).setPSetLayouts(&descriptor.layout);

    depackReturnValue(pipelineLayout, device.createPipelineLayout(pipelineLayoutInfo));

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.setStageCount(2)
        .setPStages(shaderStages)
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

    device.destroy(fragShaderModule);
    device.destroy(vertShaderModule);
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
    auto indices = QueueFamilyIndices(physicalDevice, &surface);

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(indices.graphicsFamily.value());

    depackReturnValue(commandPool, device.createCommandPool(poolInfo));
}

void VulkanGraphics::createColorResources() {
    auto colorFormat = swapchainR.format;

    auto [imageInfo, viewInfo, memPro, reqPro] = createImageInfos(
        extent.width,
        extent.height,
        1,
        msaaSamples,
        colorFormat,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::ImageAspectFlagBits::eColor);

    colorImageR.initAll(&device, imageInfo, viewInfo, memPro, reqPro);
}

void VulkanGraphics::createDepthResources() {
    auto depthFormat = findDepthFormat();
    auto [imageInfo, viewInfo, memPro, reqPro] =
        createImageInfos(extent.width,
                         extent.height,
                         1,
                         msaaSamples,
                         depthFormat,
                         vk::ImageTiling::eOptimal,
                         vk::ImageUsageFlagBits::eDepthStencilAttachment,
                         vk::MemoryPropertyFlagBits::eDeviceLocal,
                         vk::ImageAspectFlagBits::eDepth);

    depthImageR.initAll(&device, imageInfo, viewInfo, memPro, reqPro);
}

void VulkanGraphics::createTextureImage() {
    File::TextureFile tex{"Resources/Textures/viking_room.png"};
    auto              texContent = tex.read();
    auto              pixels     = texContent->pixels;
    int               texHeight = texContent->texHeight, texWidth = texContent->texWidth;
    vk::DeviceSize    imageSize = texHeight * texWidth * 4;
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    std::span<std::byte> data(reinterpret_cast<std::byte*>(pixels), imageSize);
    StagingBuffer        stagingBuffer{&device, data, phyMemPro};
    tex.free();

    auto [imageInfo, viewInfo, memPro, reqPro] =
        createImageInfos(texWidth,
                         texHeight,
                         mipLevels,
                         vk::SampleCountFlagBits::e1,
                         vk::Format::eR8G8B8A8Srgb,
                         vk::ImageTiling::eOptimal,
                         vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                         vk::MemoryPropertyFlagBits::eDeviceLocal,
                         vk::ImageAspectFlagBits::eColor);
    textureImageR.initAll(&device, imageInfo, viewInfo, memPro, reqPro);

    transitionImageLayout(textureImageR.image,
                          vk::Format::eR8G8B8A8Srgb,
                          vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eTransferDstOptimal,
                          mipLevels);
    stagingBuffer.copyTo(
        &textureImageR,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight),
        [this](std::function<void(vk::CommandBuffer&)> func) { this->disposableCommands(func); });

    generateMipmaps(textureImageR.image, vk::Format::eR8G8B8A8Srgb, texWidth, texHeight, mipLevels);
}

void VulkanGraphics::createTextureSampler() {
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.setMagFilter(vk::Filter::eLinear)
        .setMinFilter(vk::Filter::eLinear)
        .setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setAnisotropyEnable(vk::True)
        .setMaxAnisotropy(physicalDevice.getProperties().limits.maxSamplerAnisotropy)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
        .setUnnormalizedCoordinates(vk::False)
        .setCompareEnable(vk::False)
        .setCompareOp(vk::CompareOp::eAlways)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setMaxLod(static_cast<float>(mipLevels));

    depackReturnValue(textureSampler, device.createSampler(samplerInfo));
}

void VulkanGraphics::loadModel() {
    model.read();
    auto           vertices   = model.getVertices();
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    std::span<std::byte> verData(reinterpret_cast<std::byte*>(vertices.data()), bufferSize);
    vertexBufferRC.init(
        &device,
        verData,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        phyMemPro,
        [this](std::function<void(vk::CommandBuffer&)> func) { this->disposableCommands(func); });

    auto indices = model.getIndices();
    bufferSize   = sizeof(indices[0]) * indices.size();

    std::span<std::byte> idxData(reinterpret_cast<std::byte*>(indices.data()), bufferSize);
    indexBufferRC.init(
        &device,
        idxData,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        phyMemPro,
        [this](std::function<void(vk::CommandBuffer&)> func) { this->disposableCommands(func); });

    // cannot apply model.free() here.
}

void VulkanGraphics::createUniformBuffers() {
    vk::DeviceSize bufferSize = sizeof(Math::DataFormat::UniformBufferObject);

    uniformBufferRs.resize(MAX_FRAMES_IN_FLIGHT);
    std::for_each(uniformBufferRs.begin(),
                  uniformBufferRs.end(),
                  [this, bufferSize](BufferResourceUniform& unibuf) {
                      unibuf.init(&this->device, bufferSize, this->phyMemPro);
                  });
}

void VulkanGraphics::createDescriptorPool() {
    std::array<vk::DescriptorPoolSize, 2> poolSizes{};
    poolSizes[0]
        .setType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    poolSizes[1]
        .setType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

    descriptor.initPool(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), poolSizes);
}

void VulkanGraphics::createDescriptorSets() {
    descriptor.initSets(uniformBufferRs, textureSampler, textureImageR.imageView);
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
    auto bufferSize = window->getFramebufferSize();
    while (bufferSize.width == 0 || bufferSize.height == 0) {
        bufferSize = window->getFramebufferSize();
        window->waitEvents();
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

bool VulkanGraphics::isDeviceSuitable(const vk::PhysicalDevice& device) {
    QueueFamilyIndices indices = QueueFamilyIndices(device, &surface);

    auto extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport{device, surface};
        swapChainAdequate =
            !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    auto supportedFeatures = device.getFeatures();

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
                                     descriptor.sets[currentFrame],
                                     static_cast<uint32_t>(0));

    commandBuffer.drawIndexed(static_cast<uint32_t>(model.getIndices().size()), 1, 0, 0, 0);

    commandBuffer.endRenderPass();
    handleVkResult(commandBuffer.end());
}

vk::Format VulkanGraphics::findSupportedFormat(const std::vector<vk::Format>& candidates,
                                               vk::ImageTiling                tiling,
                                               vk::FormatFeatureFlags         features) {
    for (vk::Format format : candidates) {
        auto props = physicalDevice.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    logErrorMsg("failed to find supported format!");
    return {};
}

vk::Format VulkanGraphics::findDepthFormat() {
    return findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

std::tuple<vk::Buffer, vk::DeviceMemory>
VulkanGraphics::createBuffer(vk::DeviceSize          size,
                             vk::BufferUsageFlags    usage,
                             vk::MemoryPropertyFlags properties) {
    vk::Buffer       buffer{};
    vk::DeviceMemory bufferMemory{};

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

    depackReturnValue(buffer, device.createBuffer(bufferInfo));

    auto memRequirements = device.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(findMemoryType(phyMemPro, memRequirements.memoryTypeBits, properties));

    depackReturnValue(bufferMemory, device.allocateMemory(allocInfo));

    handleVkResult(device.bindBufferMemory(buffer, bufferMemory, 0));

    return std::make_tuple(std::move(buffer), std::move(bufferMemory));
}

std::tuple<vk::ImageCreateInfo,
           vk::ImageViewCreateInfo,
           vk::PhysicalDeviceMemoryProperties,
           vk::MemoryPropertyFlags>
VulkanGraphics::createImageInfos(uint32_t                width,
                                 uint32_t                height,
                                 uint32_t                mipLevels_,
                                 vk::SampleCountFlagBits numSamples,
                                 vk::Format              format,
                                 vk::ImageTiling         tiling,
                                 vk::ImageUsageFlags     usage,
                                 vk::MemoryPropertyFlags properties,
                                 vk::ImageAspectFlags    aspectFlags) {
    vk::ImageCreateInfo imageInfo{};
    imageInfo.setImageType(vk::ImageType::e2D)
        .setExtent({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1})
        .setMipLevels(mipLevels_)
        .setArrayLayers(1)
        .setFormat(format)
        .setTiling(tiling)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setUsage(usage)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setSamples(numSamples);

    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange.setAspectMask(aspectFlags)
        .setBaseMipLevel(0)
        .setLevelCount(mipLevels_)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.setViewType(vk::ImageViewType::e2D)
        .setFormat(format)
        .setSubresourceRange(subresourceRange);

    return std::make_tuple(imageInfo, viewInfo, phyMemPro, properties);
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

void VulkanGraphics::transitionImageLayout(vk::Image       image,
                                           vk::Format      format,
                                           vk::ImageLayout oldLayout,
                                           vk::ImageLayout newLayout,
                                           uint32_t        mipLevels_) {
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange.setBaseMipLevel(0)
        .setLevelCount(mipLevels_)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

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

    disposableCommands(func);
}

void VulkanGraphics::copyBufferToImage(vk::Buffer buffer,
                                       vk::Image  image,
                                       uint32_t   width,
                                       uint32_t   height) {
    vk::ImageSubresourceLayers subresource{};
    subresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setMipLevel(0)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    vk::BufferImageCopy region{};
    region.setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(0)
        .setImageSubresource(subresource)
        .setImageOffset({0, 0, 0})
        .setImageExtent({width, height, 1});

    disposableCommands([&buffer, &image, &region](vk::CommandBuffer& cmdBuffer) {
        cmdBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
    });
}

void VulkanGraphics::generateMipmaps(vk::Image  image,
                                     vk::Format imageFormat,
                                     int32_t    texWidth,
                                     int32_t    texHeight,
                                     uint32_t   mipLevels_) {
    auto formatProperties = physicalDevice.getFormatProperties(imageFormat);
    if (!(formatProperties.optimalTilingFeatures &
          vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        logErrorMsg("texture image format does not support linear blitting!");
    }

    disposableCommands([&image, &texWidth, &texHeight, &mipLevels_](
                           const vk::CommandBuffer& cmdBuffer) {
        vk::ImageMemoryBarrier barrier{};
        barrier.image                           = image;
        barrier.srcQueueFamilyIndex             = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex             = vk::QueueFamilyIgnored;
        barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        barrier.subresourceRange.levelCount     = 1;

        int32_t mipWidth = texWidth, mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels_; i++) {
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

            cmdBuffer.blitImage(image,
                                vk::ImageLayout::eTransferSrcOptimal,
                                image,
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
            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }
        barrier.subresourceRange.baseMipLevel = mipLevels_ - 1;
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

vk::SampleCountFlagBits VulkanGraphics::getMaxUsableSampleCount() {
    auto physicalDeviceProperties = physicalDevice.getProperties();

    vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                  physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; };
    if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; };
    if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; };
    if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; };
    if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; };
    if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; };
    return vk::SampleCountFlagBits::e1;
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
