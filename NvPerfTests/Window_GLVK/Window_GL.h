#pragma once
#include "glad/gl.h"
#include "glad/glx.h" // X11
#include "glad/egl.h" // Wayland
#include <GLFW/glfw3.h>
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
    double EndGPUTimer(bool print);

protected:
    GLFWwindow* m_window = NULL;
    GLuint m_query1 = 0, m_query2 = 0;
    double m_accumTimeMs = 0.0, m_accumFrames = 0.0;
    std::chrono::high_resolution_clock::time_point m_printTime;
    bool m_waitPrevTimer = false;
};