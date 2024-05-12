#pragma once

#include "TBEngine/graphics/graphics.hpp"
#include "TBEngine/window/window.hpp"
#include "TBEngine/ui/ui.hpp"

namespace TBE::Engine {

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
    Window::Window           winForm;
    Graphics::VulkanGraphics graphic;
    Ui::Ui                   ui;
};

} // namespace TBE::Engine
