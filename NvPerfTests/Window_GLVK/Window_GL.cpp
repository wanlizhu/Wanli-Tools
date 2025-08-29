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

double Window_GL::EndGPUTimer() {
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
            if (time - m_printTime > std::chrono::seconds(2)) {
                double fps = 1000.0 / (m_accumTimeMs / m_accumFrames);
                m_accumTimeMs = 0.0;
                m_accumFrames = 0.0;
                m_printTime = time;
                return fps;
            }
        }
    } else {
        glQueryCounter(m_query2, GL_TIMESTAMP);
        m_waitPrevTimer = true;
        return EndGPUTimer();
    }

    return -1.0;
}

GLuint CompileShader(GLenum stage, const std::string& filename) {
    const char* source = nullptr;
    if (std::filesystem::exists(filename)) {
        std::cout << "Load shader from file: " << filename << std::endl;
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << filename << std::endl;
            throw std::runtime_error("Cannot open file");
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string bufferstr = buffer.str();
        source = bufferstr.c_str();
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
    std::cout << "Linked program " << program << " with " << vsfile << " and " << psfile << std::endl;

    return program;
}