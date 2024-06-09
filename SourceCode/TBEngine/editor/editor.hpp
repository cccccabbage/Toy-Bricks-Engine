#pragma once

#include "ui/ui.hpp"
#include "delegateManager/delegateManager.hpp"
#include "TBEngine/utils/macros/includeVulkan.hpp"

namespace TBE::Editor {

class Editor : public Ui::Ui, public DelegateManager::DelegateManager {
public:
    Editor(ImGui_ImplVulkan_InitInfo imguiInfo, GLFWwindow* pWindow_);
    ~Editor();

public:
    void tickGPU(const vk::CommandBuffer& cmdBuffer);
    void tickCPU();
};

} // namespace TBE::Editor
