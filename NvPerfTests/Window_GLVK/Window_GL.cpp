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

    glGenQueries(1, &m_query1);
    glGenQueries(1, &m_query2);
    m_printTime = std::chrono::high_resolution_clock::now();

    std::cout << "Window Initialization Finished" << std::endl;
}

void Window_GL::CloseWindow() {
    glDeleteQueries(1, &m_query1);
    glDeleteQueries(1, &m_query2);
    glfwDestroyWindow(m_window);
    glfwTerminate();
    std::cout << "Window Cleanup Finished" << std::endl;
}

void Window_GL::BeginGPUTimer() {
    if (!m_waitPrevTimer) {
        glQueryCounter(m_query1, GL_TIMESTAMP);
    }
}

double Window_GL::EndGPUTimer(bool print) {
    if (m_waitPrevTimer) {
        GLint a0 = 0, a1 = 0;
        glGetQueryObjectiv(m_query1, GL_QUERY_RESULT_AVAILABLE, &a0);
        glGetQueryObjectiv(m_query2, GL_QUERY_RESULT_AVAILABLE, &a1);
        if (a0 && a1) {
            m_waitPrevTimer = false;
            uint64_t t0 = 0, t1 = 0;
            glGetQueryObjectui64v(m_query1, GL_QUERY_RESULT, &t0);
            glGetQueryObjectui64v(m_query2, GL_QUERY_RESULT, &t1);
            const uint64_t ns = (t1 >= t0) ? (t1 - t0) : 0; 
            double ms = ns * 1.0 / 1e6;
            m_accumTimeMs += ms;
            m_accumFrames += 1;

            auto time = std::chrono::high_resolution_clock::now();
            if (print && (time - m_printTime > std::chrono::seconds(2))) {
                double fps = 1000.0 / (m_accumTimeMs / m_accumFrames);
                printf("Avg FPS: %.2f\n", fps);
                m_accumTimeMs = 0.0;
                m_accumFrames = 0.0;
                m_printTime = time;
            }
            return ms; // ms
        }
        return -1.0;
    } else {
        glQueryCounter(m_query2, GL_TIMESTAMP);
        m_waitPrevTimer = true;
        return EndGPUTimer(print);
    }
}