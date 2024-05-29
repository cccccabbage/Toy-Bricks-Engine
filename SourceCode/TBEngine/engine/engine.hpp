#pragma once

#include "TBEngine/core/graphics/graphics.hpp"
#include "TBEngine/core/window/window.hpp"
#include "TBEngine/editor/editor.hpp"

namespace TBE::Engine
{

class Engine
{
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
    void captureKeyInput(TBE::Editor::DelegateManager::KeyType keyType) {
        switch(keyType) {
            case TBE::Editor::DelegateManager::KeyType::eEscape :
                shouldClose = true;
                break;
            default:
                break;
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
