#pragma once

#include "TBEngine/graphics/graphics.hpp"
#include "TBEngine/window/window.hpp"

namespace TBE::Engine {

class Engine {
public:
    Engine();

public:
    void init();
    void runLoop();
    void exit();

private:
    void tick();

private:
    Window::Window           winForm{{1280, 720}};
    Graphics::VulkanGraphics graphic{&winForm};
};

} // namespace TBE::Engine
