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
    double EndGPUTimer(bool wait);
    double ReadTimer();
    double ReadAndResetTimer();
    GLuint CompileAndLinkShaders(
        const std::string& vsfile, 
        const std::string& psfile
    );

protected:
    GLFWwindow* m_window = NULL;

private:
    GLuint m_queries[2] = {0, 0};
    std::chrono::high_resolution_clock::time_point m_timePoint;
    int m_debugCounter = 0;
};