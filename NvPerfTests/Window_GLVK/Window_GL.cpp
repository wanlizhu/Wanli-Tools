#include "Window_GL.h"

#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#include <GLFW/glfw3native.h>
#include <X11/Xlib.h>  

void GLFW_ErrorCallback_GL(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void Window_GL::OpenWindow(
    const char* title, 
    int width, int height, 
    const std::map<int, int>& hints
) {
    glfwSetErrorCallback(GLFW_ErrorCallback_GL);
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    for (auto& [key, val] : hints) {
        glfwWindowHint(key, val);
    }

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoaderLoadGL()) {
        glfwTerminate();
        throw std::runtime_error("Failed to call gladLoaderLoadGL");
    }

    Display* display = glfwGetX11Display();
    if (!gladLoaderLoadGLX(display, DefaultScreen(display))) {
        glfwTerminate();
        throw std::runtime_error("Failed to call gladLoaderLoadGLX");
    }

    glfwSwapInterval(0); // Disable Vsync

    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);
    std::cout << "OpenGL Vendor: " << vendor << std::endl;
    std::cout << "OpenGL Renderer: " << renderer << std::endl;
    std::cout << "OpenGL Version: " << version << std::endl;

    std::cout << "Window Initialization Finished" << std::endl;
}

void Window_GL::CloseWindow() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
    std::cout << "Window Cleanup Finished" << std::endl;
}