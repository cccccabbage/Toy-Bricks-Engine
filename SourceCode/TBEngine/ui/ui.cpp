#include "ui.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace TBE::Ui {
using namespace ImGui;

constexpr uint32_t max_descriptor_set_for_texture = 100;

void CheckVkResult(VkResult result) { handleVkResult((vk::Result)result); }

Ui::Ui(UiCreateInfo createInfo)
    : device(createInfo.device), phyDevice(createInfo.phyDevice), descriptor(device, phyDevice) {
    initVulkanMembers(createInfo);
    initImgui(createInfo);
}

Ui::~Ui() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    DestroyContext();
}

void Ui::initImgui(UiCreateInfo& createInfo) {
    IMGUI_CHECKVERSION();
    CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(createInfo.window, true);
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance        = createInfo.instance;
    init_info.PhysicalDevice  = createInfo.phyDevice;
    init_info.Device          = createInfo.device;
    init_info.QueueFamily     = createInfo.queueFamilyIndex;
    init_info.Queue           = createInfo.queue;
    init_info.PipelineCache   = nullptr;
    init_info.DescriptorPool  = descriptor.pool;
    init_info.RenderPass      = renderPass;
    init_info.Subpass         = 0;
    init_info.MinImageCount   = createInfo.minImageCount;
    init_info.ImageCount      = createInfo.minImageCount;
    init_info.MSAASamples     = (VkSampleCountFlagBits)vk::SampleCountFlagBits::e1;
    init_info.Allocator       = nullptr;
    init_info.CheckVkResultFn = CheckVkResult;

    ImGui_ImplVulkan_Init(&init_info);
}

void Ui::initVulkanMembers(UiCreateInfo& createInfo) {
    descriptor.initLayout(std::array{vk::DescriptorSetLayoutBinding{}});
    vk::DescriptorPoolSize poolSize(vk::DescriptorType::eCombinedImageSampler, 1);
    std::array             poolSizes = {poolSize};
    descriptor.initPool(max_descriptor_set_for_texture + 1, poolSizes);
    vk::DescriptorSetAllocateInfo                                       allocInfo{};
    std::array<vk::DescriptorSetLayout, max_descriptor_set_for_texture> layouts;
    layouts.fill(descriptor.layout);
    allocInfo.setDescriptorPool(descriptor.pool)
        .setDescriptorSetCount(max_descriptor_set_for_texture)
        .setSetLayouts(layouts);
    depackReturnValue(descriptor.sets, device.allocateDescriptorSets(allocInfo));

    { // init renderpass
        vk::AttachmentDescription attachment{};
        attachment.setFormat(createInfo.swapchainFormat)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        vk::AttachmentReference color_attachment{};
        color_attachment.setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpass;
        subpass.setColorAttachments(color_attachment);

        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        vk::RenderPassCreateInfo info{};
        info.setAttachmentCount(1)
            .setAttachments(attachment)
            .setSubpasses(subpass)
            .setDependencies(dependency);

        depackReturnValue(renderPass, device.createRenderPass(info));
    }

    { // init framebuffer
        vk::FramebufferCreateInfo info{};
        info.setRenderPass(renderPass)
            .setWidth(createInfo.width)
            .setHeight(createInfo.height)
            .setLayers(1);
        framebuffers.resize(createInfo.minImageCount);
        for (uint32_t i = 0; i < 2; i++) {
            std::array<vk::ImageView, 1> views{createInfo.swapchainImageViews[i]};
            info.setAttachments(views);
            depackReturnValue(framebuffers[i], device.createFramebuffer(info));
        }
    }

    { // init combined sampler
        vk::SamplerCreateInfo info{};
        depackReturnValue(combinedSampler, device.createSampler(info));
    }

    { // init cmd pool and buffer
        vk::CommandPoolCreateInfo createInfo{};
        createInfo.setQueueFamilyIndex(createInfo.queueFamilyIndex);
        depackReturnValue(cmdPool, device.createCommandPool(createInfo));

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.setCommandPool(cmdPool).setCommandBufferCount(1).setLevel(
            vk::CommandBufferLevel::ePrimary);

        std::vector<vk::CommandBuffer> buffers;
        depackReturnValue(buffers, device.allocateCommandBuffers(allocInfo));
        cmdBuf = buffers[0];
    }
}

void Ui::tick() {
    handleVkResult(device.resetCommandPool(cmdPool));
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    handleVkResult(cmdBuf.begin(beginInfo));

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    NewFrame();

    bool show = true;
    if (show) ShowDemoWindow(&show);

    Render();

    ImDrawData* main_draw_data = GetDrawData();

    vk::RenderPassBeginInfo renderPassInfo{};
    vk::ClearValue          clearValue{vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f}};
    renderPassInfo.setRenderPass(renderPass)
        .setFramebuffer(framebuffers[curDescriptorSetIndex])
        .setRenderArea({{0, 0}, {1280, 720}}) // TODO: width, height
        .setClearValues(clearValue);
    cmdBuf.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    ImGui_ImplVulkan_RenderDrawData(main_draw_data, cmdBuf);

    cmdBuf.endRenderPass();

    handleVkResult(cmdBuf.end());
}

} // namespace TBE::Ui
