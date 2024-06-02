#pragma once

#include "TBEngine/core/graphics/graphics.hpp"
#include "TBEngine/core/window/window.hpp"
#include "TBEngine/editor/editor.hpp"

namespace TBE::Engine {
using TBE::Editor::DelegateManager::KeyBit;
using TBE::Editor::DelegateManager::KeyStateMap;

class Engine {
public:
    Engine();
    ~Engine();

public:
    void runLoop();

private:
    void init();
    void tick();
    void exit();

private:
    void captureKeyInput(KeyStateMap keyMap) {
        if (keyMap & (KeyStateMap)(KeyBit::eEscape)) {
            shouldClose = true;
        }
    }

private:
    Window::Window           winForm;
    Graphics::VulkanGraphics graphic;
    Editor::Editor           editor;

private:
    bool shouldClose = false;
};

} // namespace TBE::Engine
