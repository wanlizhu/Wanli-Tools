#pragma once
#include "glad/gl.h"
#include "glad/glx.h" // X11
#include "glad/egl.h" // Wayland
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <map>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <unordered_map>
#include <chrono>
#include <unistd.h>
#include <cstdio>

static constexpr int _sizeof_(GLenum color) {
    switch (color) {
        case GL_RGBA8:  return 4;
        case GL_RGBA16: return 8;
        case GL_RGBA32UI: return 16;
        case GL_RGBA32F: return 16;
        default: assert(false); return 0;
    }
}

static constexpr const char* _str_(GLenum color) {
    switch (color) {
        case GL_RGBA8:  return "GL_RGBA8";
        case GL_RGBA16: return "GL_RGBA16";
        case GL_RGBA32UI: return "GL_RGBA32UI";
        case GL_RGBA32F: return "GL_RGBA32F";
        default: assert(false); return "";
    }
}

static constexpr int _type_(GLenum color) {
    switch (color) {
        case GL_RGBA8:  return GL_UNSIGNED_BYTE;
        case GL_RGBA16: return GL_UNSIGNED_SHORT;
        case GL_RGBA32UI: return GL_UNSIGNED_INT;
        case GL_RGBA32F: return GL_FLOAT;
        default: assert(false); return 0;
    }
}

inline const char* _size_(uint64_t size) {
    static char buffer[32];
    if (size >= 1024ULL * 1024 * 1024) {
        double gb = size / (1024.0 * 1024.0 * 1024.0);
        snprintf(buffer, sizeof(buffer), "%.2f GB", gb);
    } else if (size >= 1024ULL * 1024) {
        double mb = size / (1024.0 * 1024.0);
        snprintf(buffer, sizeof(buffer), "%.2f MB", mb);
    } else if (size >= 1024ULL) {
        double kb = size / 1024.0;
        snprintf(buffer, sizeof(buffer), "%.2f KB", kb);
    } else {
        snprintf(buffer, sizeof(buffer), "%llu B", (unsigned long long)size);
    }
    return buffer;
}

extern std::unordered_map<std::string, std::vector<uint8_t>> g_embeddedFiles;

class Window_GL {
public:
    virtual ~Window_GL() {}

protected:
    void OpenWindow(
        const char* title, 
        int width, int height, 
        const std::map<int, int>& hints
    );
    void CloseWindow(); 
    void BeginGPUTimer();
    void EndGPUTimer();
    std::chrono::nanoseconds ReadGPUTimer();
    std::chrono::nanoseconds ReadCPUTimer();
    std::chrono::nanoseconds ResetCPUTimer();
    GLuint CompileAndLinkShaders(
        const std::string& vsfile, 
        const std::string& psfile
    );

protected:
    GLFWwindow* m_window = NULL;

private:
    GLuint m_queries[2] = {0, 0};
    std::chrono::high_resolution_clock::time_point m_timePoint;
};