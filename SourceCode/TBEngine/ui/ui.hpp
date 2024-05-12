#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/graphics/vulkanAbstract/descriptor/descriptor.hpp"
#include "TBEngine/graphics/vulkanAbstract/renderPass/renderPass.hpp"

#include <GLFW/glfw3.h>
#include <vector>

namespace TBE::Ui {

struct UiCreateInfo {
    GLFWwindow*                window = nullptr;
    vk::Instance               instance{};
    vk::PhysicalDevice         phyDevice{};
    vk::Device                 device{};
    uint32_t                   queueFamilyIndex{};
    vk::Queue                  queue{};
    uint32_t                   minImageCount{};
    vk::Format                 swapchainFormat{};
    std::vector<vk::ImageView> swapchainImageViews{};
    uint32_t                   width{};
    uint32_t                   height{};
};

class Ui {
public:
    Ui(UiCreateInfo createInfo);
    ~Ui();

public:
    Graphics::Descriptor descriptor;

    vk::RenderPass               renderPass;
    vk::Sampler                  combinedSampler;
    std::vector<vk::Framebuffer> framebuffers;
    vk::CommandPool              cmdPool;
    vk::CommandBuffer            cmdBuf;

    size_t curDescriptorSetIndex = 0;

public:
    void tick();

private:
    void initImgui(UiCreateInfo& createInfo);
    void initVulkanMembers(UiCreateInfo& createInfo);

private:
    vk::Device         device;
    vk::PhysicalDevice phyDevice;
};

} // namespace TBE::Ui
