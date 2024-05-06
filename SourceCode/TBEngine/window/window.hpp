#pragma once

#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vector>
#include <utility>


namespace TBE::Window {

std::vector<const char*> getRequiredExtensions();

struct BufferSize {
    uint32_t width  = 0;
    uint32_t height = 0;

    std::pair<uint32_t, uint32_t> toPair() const {
        return std::move(std::make_pair(width, height));
    };
};

class Window {
public:
    Window(BufferSize size) : winSize(size) {}

public:
    void init();
    void tick();
    void exit();

public:
    bool shouldClose();

public:
    void setResizeFlag(bool* pResize) { framebufferResized = pResize; }

private:
    GLFWwindow* pWindow = nullptr;
    BufferSize  winSize{};
    const char* winTitle = "Toy Bricks Engine";

    bool* framebufferResized = nullptr;

public:
    HWND       getWin32Window() const { return glfwGetWin32Window(pWindow); }
    auto       getModuleHandle() const { return GetModuleHandle(nullptr); }
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