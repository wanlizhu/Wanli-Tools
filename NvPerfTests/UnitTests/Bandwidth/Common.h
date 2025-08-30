#pragma once
#include "Window_GL.h"
#include <random>
#include <iomanip>
#include <string>
#include <map>

#define TEXTURE_WIDTH  3840
#define TEXTURE_HEIGHT 2160

struct TestResult {
    std::string testName;
    std::string operation;
    double bandwidth_gbps;
    double fps;
};
