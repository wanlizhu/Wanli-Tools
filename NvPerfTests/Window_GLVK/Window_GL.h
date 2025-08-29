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

class Window_GL {
public:
    virtual ~Window_GL() {}

protected:
    virtual void OpenWindow(
        const char* title, 
        int width, int height, 
        const std::map<int, int>& hints
    );
    virtual void CloseWindow(); 

protected:
    GLFWwindow* m_window = NULL;
};