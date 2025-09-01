#pragma once
#include <deque>
#include <dlfcn.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <optional>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <functional>
#include <chrono>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <atomic>

struct GPUTimerRec {
    std::string name;
    uint64_t timeInTotal = 0; // nanoseconds
    uint64_t runsInTotal = 0;
    std::chrono::high_resolution_clock::time_point printTime;

    GPUTimerRec() {
        printTime = std::chrono::high_resolution_clock::now();
    }
};

void RunWithGPUTimer(
    const std::string& name, 
    const std::function<void()>& func
);
void RunAsync(std::function<void()> task);

extern "C" void glBlitFramebuffer(
    GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
    GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
    GLbitfield mask, GLenum filter
);