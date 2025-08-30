#include "Window_GL.h"
#include <chrono>
#include <cstdint>
#include <ratio>
#include <unistd.h>

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

    glGenQueries(2, m_queries);
    m_timePoint = std::chrono::high_resolution_clock::now();

    std::cout << "Created Window \"" << title <<  "\" " << width << "x" << height << std::endl;
}

void Window_GL::CloseWindow() {
    glDeleteQueries(2, m_queries);
    glfwDestroyWindow(m_window);
    glfwTerminate();
    std::cout << "Window Cleanup Finished" << std::endl;
}

void Window_GL::BeginGPUTimer() {
    glQueryCounter(m_queries[0], GL_TIMESTAMP);
}

void Window_GL::EndGPUTimer() {
    glQueryCounter(m_queries[1], GL_TIMESTAMP);
}

std::chrono::nanoseconds Window_GL::ReadGPUTimer() {
    GLint available[2] = {0, 0};
    int retries = 0;
    do {
        glGetQueryObjectiv(m_queries[0], GL_QUERY_RESULT_AVAILABLE, &available[0]);
        glGetQueryObjectiv(m_queries[1], GL_QUERY_RESULT_AVAILABLE, &available[1]);
        if (++retries > 1) {
            usleep(100);
        }
    } while (!(available[0] && available[1]));
    assert(available[0] && available[1]);
    
    GLuint64 startTime = 0, endTime = 0;
    glGetQueryObjectui64v(m_queries[0], GL_QUERY_RESULT, &startTime);
    glGetQueryObjectui64v(m_queries[1], GL_QUERY_RESULT, &endTime);
    assert(endTime >= startTime);

    return std::chrono::nanoseconds(endTime - startTime);
}

std::chrono::nanoseconds Window_GL::ReadCPUTimer() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_timePoint);
}

std::chrono::nanoseconds Window_GL::ResetCPUTimer() {
    auto ns = ReadCPUTimer();
    m_timePoint = std::chrono::high_resolution_clock::now();
    return ns;
}

GLuint CompileShader(GLenum stage, const std::string& filename) {
    const char* source = nullptr;
    std::string fileContent;
    
    if (std::filesystem::exists(filename)) {
        std::cout << "Load shader from file: " << filename << std::endl;
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << filename << std::endl;
            throw std::runtime_error("Cannot open file");
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        fileContent = buffer.str();
        source = fileContent.c_str();
    }
    else {
        std::cout << "Load shader from embedded string: " << filename << std::endl;
        source = (const char*)g_embeddedFiles.at(filename).data();
    }

    GLuint shader = glCreateShader(stage);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int result = 0, length = 0;
    char* message = nullptr;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(shader, length, &length, message);
        glDeleteShader(shader);

        std::cerr << "Failed to compile " << (stage == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader: " << message << std::endl;
        throw std::runtime_error("Failed to compile shader");
    }

    return shader;
}

GLuint Window_GL::CompileAndLinkShaders(
    const std::string& vsfile, 
    const std::string& psfile 
) {
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsfile);
    GLuint ps = CompileShader(GL_FRAGMENT_SHADER, psfile);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, ps);
    glLinkProgram(program);

    int result = 0, length = 0;
    char* message = nullptr;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        message = (char*)alloca(length * sizeof(char));
        glGetProgramInfoLog(program, length, &length, message);
        glDeleteProgram(program);

        std::cerr << "Failed to link program: " << message << std::endl;
        throw std::runtime_error("Failed to link program");
    }

    glDeleteShader(vs);
    glDeleteShader(ps);
    std::cout << "Linked program (ID: " << program << ") with " << vsfile << " and " << psfile << std::endl;

    return program;
}