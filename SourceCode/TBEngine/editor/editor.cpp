#include "editor.hpp"

namespace TBE::Editor {
using TBE::Editor::DelegateManager::KeyType;
using TBE::Editor::DelegateManager::DelegateIndexGetter;

Editor::Editor(ImGui_ImplVulkan_InitInfo imguiInfo, GLFWwindow *pWindow_)
    : Ui::Ui(imguiInfo, pWindow_)
    , DelegateManager::DelegateManager() {}

Editor::~Editor() {}

void Editor::tickGPU(const vk::CommandBuffer& cmdBuffer) {
    Ui::Ui::tickGPU(cmdBuffer);
}

void Editor::tickCPU() {
    auto key = ImGuiKey_Escape;
    if (ImGui::IsKeyDown(key)) {
        boardcast<KeyType>(DelegateIndexGetter::get<KeyType>(), KeyType::eEscape);
    }
}

}
