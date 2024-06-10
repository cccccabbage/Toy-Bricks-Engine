#pragma once

#include "TBEngine/core/graphics/graphics.hpp"
#include "TBEngine/core/window/window.hpp"
#include "TBEngine/editor/editor.hpp"

namespace TBE::Engine {
using TBE::Editor::DelegateManager::KeyStateMap;

class Engine {
public:
    Engine();
    ~Engine();

public:
    void runLoop();

private:
    Window::Window           winForm;
    Graphics::VulkanGraphics graphic;
    Editor::Editor           editor;

private:
    bool shouldClose = false;

private:
    void tick();

private:
    void bindTickGPUFuncs();
    void bindCallBackFuncs();

private:
    void captureKeyInput(KeyStateMap keyMap) {
        if (keyMap & (KeyStateMap)(KeyBit::eEscape)) {
            shouldClose = true;
        }
    }
};

} // namespace TBE::Engine
