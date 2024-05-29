#include "ui.hpp"

#include "TBEngine/core/graphics/graphics.hpp"

#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>


namespace TBE::Editor::Ui {
using Graphics::VulkanGraphics;

inline void checkResult(VkResult res) {
    handleVkResult((vk::Result)res);
}

Ui::Ui(ImGui_ImplVulkan_InitInfo imguiInfo, GLFWwindow *pWindow) {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui_ImplGlfw_InitForVulkan(pWindow, true);

    vk::DescriptorPoolSize poolSize{};
    poolSize.setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1);
    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.setPoolSizes(poolSize).setMaxSets(1);
    depackReturnValue(descPool, VulkanGraphics::device.createDescriptorPool(poolInfo));

    imguiInfo.DescriptorPool  = descPool;
    imguiInfo.CheckVkResultFn = checkResult;

    ImGui_ImplVulkan_Init(&imguiInfo);
}

Ui::~Ui() {
    VulkanGraphics::device.destroy(descPool);
}

void Ui::tickGPU(const vk::CommandBuffer& cmdBuffer) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawData, cmdBuffer);
}

} // namespace TBE::Ui
