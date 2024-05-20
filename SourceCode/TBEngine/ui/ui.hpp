#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"

#include <tuple>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

namespace TBE::Ui
{

class Ui
{
public:
    void init(std::tuple<ImGui_ImplVulkan_InitInfo, GLFWwindow*> info);
    void destroy();

public:
    void tick(const vk::CommandBuffer& cmdBuffer);

private:
    vk::DescriptorPool descPool{};
};

} // namespace TBE::Ui
