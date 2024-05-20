#include "ui.hpp"

#include "TBEngine/core/graphics/graphics.hpp"

#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>


namespace TBE::Ui
{
using Graphics::VulkanGraphics;

inline void checkResult(VkResult res)
{
    handleVkResult((vk::Result)res);
}

void Ui::init(std::tuple<ImGui_ImplVulkan_InitInfo, GLFWwindow*> Info)
{
    auto [info, pWindow] = Info;

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui_ImplGlfw_InitForVulkan(pWindow, true);

    vk::DescriptorPoolSize poolSize{};
    poolSize.setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1);
    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.setPoolSizes(poolSize).setMaxSets(1);
    depackReturnValue(descPool, VulkanGraphics::device.createDescriptorPool(poolInfo));

    info.DescriptorPool  = descPool;
    info.CheckVkResultFn = checkResult;

    ImGui_ImplVulkan_Init(&info);
}

void Ui::destroy()
{
    VulkanGraphics::device.destroy(descPool);
}

void Ui::tick(const vk::CommandBuffer& cmdBuffer)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawData, cmdBuffer);
}

} // namespace TBE::Ui
