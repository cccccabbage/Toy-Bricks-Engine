#include "editor.hpp"
#include "imgui.h"
#include "TBEngine/core/graphics/graphics.hpp"

namespace TBE::Editor {
using TBE::Editor::DelegateManager::KeyStateMap;
using TBE::Editor::DelegateManager::DelegateIndexGetter;
using TBE::Editor::DelegateManager::KeyBit;

const std::unordered_map<ImGuiKey, KeyBit> keyCaptureList = {
    {ImGuiKey_Escape,     KeyBit::eEscape},
    {ImGuiKey_W,          KeyBit::eW},
    {ImGuiKey_A,          KeyBit::eA},
    {ImGuiKey_S,          KeyBit::eS},
    {ImGuiKey_D,          KeyBit::eD},
    {ImGuiKey_LeftCtrl,   KeyBit::eLeftCtrl},
    {ImGuiKey_Space,      KeyBit::eSpace},
    {ImGuiKey_LeftShift,  KeyBit::eLeftShift},
    {ImGuiKey_LeftArrow,  KeyBit::eLeft},
    {ImGuiKey_RightArrow, KeyBit::eRight},
    {ImGuiKey_UpArrow,    KeyBit::eUp},
    {ImGuiKey_DownArrow,  KeyBit::eDown},
};

Editor::Editor(ImGui_ImplVulkan_InitInfo imguiInfo, GLFWwindow *pWindow_)
    : Ui::Ui(imguiInfo, pWindow_)
    , DelegateManager::DelegateManager() {}

Editor::~Editor() {}

void Editor::tickGPU(const vk::CommandBuffer& cmdBuffer) {
    Ui::Ui::tickGPU(cmdBuffer);
}

void Editor::tickCPU() {
    KeyStateMap keyMap = (KeyStateMap)KeyBit::eNull;
    for (const auto& [imguiKey, keyBit] : keyCaptureList) {
        if (ImGui::IsKeyDown(imguiKey)) {
            keyMap |= (KeyStateMap) keyBit;
        }
    }
    if (keyMap != (KeyStateMap)KeyBit::eNull) { // key on capture list is pressed
        boardcast(DelegateIndexGetter::get<KeyStateMap>(), keyMap);
    }
}

}
