#pragma once

#include "TBEngine/utils/includes/includeGLFW.hpp"

#include <vector>
#include <utility>


namespace TBE::Window {

std::vector<const char*> getRequiredExtensions();

struct BufferSize {
    uint32_t width  = 0;
    uint32_t height = 0;

    operator std::pair<uint32_t, uint32_t>() { return std::move(std::make_pair(width, height)); }
};

class Window {
public:
    Window(BufferSize size);
    ~Window();

public:
    void tick();

private:
    void init();
    void exit();

public:
    bool shouldClose();

public:
    void setResizeFlag(bool* pResize) { framebufferResized = pResize; }

private:
    GLFWwindow* pWindow = nullptr;
    BufferSize  winSize;
    const char* winTitle = "Toy Bricks Engine";

    bool* framebufferResized = nullptr;

public:
    HWND       getWin32Window() const { return glfwGetWin32Window(pWindow); }
    const auto getModuleHandle() const { return GetModuleHandle(nullptr); }
    auto       getPWindow() { return pWindow; }
    BufferSize getFramebufferSize() const {
        int width, height;
        glfwGetFramebufferSize(pWindow, &width, &height);
        BufferSize ret = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        return std::move(ret);
    }
    void waitEvents() const { glfwWaitEvents(); }


private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto owner                   = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        *(owner->framebufferResized) = true;
        owner->winSize               = owner->getFramebufferSize();
    }
};

} // namespace TBE::Window
