#pragma once

#include "TBEngine/utils/includes/includeVulkan.hpp"

#include <tuple>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

namespace TBE::Editor::Ui {

class Ui {
public:
    Ui(ImGui_ImplVulkan_InitInfo imguiInfo, GLFWwindow* pWindow);
    ~Ui();

public:
    void tickGPU(const vk::CommandBuffer& cmdBuffer);

private:
    vk::DescriptorPool descPool{};
};

} // namespace TBE::Editor::Ui
