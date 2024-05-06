#include "TBEngine/window/window.hpp"
#include "TBEngine/utils/log/log.hpp"


#ifdef NDEBUG
const bool inDebug = false;
#else
const bool inDebug = true;
#endif

extern const TBE::Utils::Log::Logger* logger;

namespace TBE::Window {

std::vector<const char*> getRequiredExtensions() {
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    return extensions;
}

void Window::init() {
    logger->trace("Initializing window.");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    pWindow = glfwCreateWindow(winSize.width, winSize.height, winTitle, nullptr, nullptr);
    glfwSetWindowUserPointer(pWindow, this);
    glfwSetFramebufferSizeCallback(pWindow, framebufferResizeCallback);

    logger->trace("Window initialized.");
}

void Window::tick() { glfwPollEvents(); }

void Window::exit() {
    glfwDestroyWindow(pWindow);
    glfwTerminate();
}

bool Window::shouldClose() { return glfwWindowShouldClose(pWindow); }


} // namespace TBE::Window
