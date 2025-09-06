#include "HookUtils.h"
#include <functional>
#include <inttypes.h>
#include <unordered_map>

__attribute__((constructor))
static void GLHooks_BEGIN() {
}

__attribute__((destructor))
static void GLHooks_END() {
}

extern "C" void (*glXGetProcAddress(const GLubyte *procname))(void) {
    static auto real_glXGetProcAddress = (decltype(&glXGetProcAddress))dlsym(RTLD_NEXT, "glXGetProcAddress");

    if (strcmp((const char*)procname, "glBlitFramebuffer") == 0) {
        return (void (*)(void)) glBlitFramebuffer;
    } else if (strcmp((const char*)procname, "glBufferSubData") == 0) {
        //return (void (*)(void)) glBufferSubData;
    } else if (strcmp((const char*)procname, "glFramebufferTexture2D") == 0) {
        //return (void (*)(void)) glFramebufferTexture2D;
    }

    return real_glXGetProcAddress(procname);
}

extern "C" void (*glXGetProcAddressARB(const GLubyte *procname))(void) {
    return glXGetProcAddress(procname);
}

void RunWithGPUTimer(
    const std::string& name,
    const std::function<void()>& func
) {
    static auto real_glXGetProcAddress = (void* (*)(const GLubyte*))dlsym(RTLD_NEXT, "glXGetProcAddress");
    static auto glGenQueries = real_glXGetProcAddress ? (PFNGLGENQUERIESPROC)real_glXGetProcAddress((const GLubyte*)"glGenQueries") : nullptr;
    static auto glQueryCounter = real_glXGetProcAddress ? (PFNGLQUERYCOUNTERPROC)real_glXGetProcAddress((const GLubyte*)"glQueryCounter") : nullptr;
    static auto glGetQueryObjectiv = real_glXGetProcAddress ? (PFNGLGETQUERYOBJECTIVPROC)real_glXGetProcAddress((const GLubyte*)"glGetQueryObjectiv") : nullptr;
    static auto glGetQueryObjectui64v = real_glXGetProcAddress ? (PFNGLGETQUERYOBJECTUI64VPROC)real_glXGetProcAddress((const GLubyte*)"glGetQueryObjectui64v") : nullptr;
    static auto glDeleteQueries = real_glXGetProcAddress ? (PFNGLDELETEQUERIESPROC)real_glXGetProcAddress((const GLubyte*)"glDeleteQueries") : nullptr;
    static std::unordered_map<std::string, GPUTimerRec> records;
    static std::vector<std::function<bool()>> deferredTasks;

    GLuint queries[2];
    glGenQueries(2, queries);

    glQueryCounter(queries[0], GL_TIMESTAMP);
    func();
    glQueryCounter(queries[1], GL_TIMESTAMP);

    deferredTasks.push_back([&, name, queries]()->bool{
        GLint a0, a1;
        glGetQueryObjectiv(queries[0], GL_QUERY_RESULT_AVAILABLE, &a0);
        glGetQueryObjectiv(queries[1], GL_QUERY_RESULT_AVAILABLE, &a1);
        if (!(a0 && a1)) {
            return false;
        }

        GLuint64 startTime = 0, endTime = 0;
        glGetQueryObjectui64v(queries[0], GL_QUERY_RESULT, &startTime);
        glGetQueryObjectui64v(queries[1], GL_QUERY_RESULT, &endTime);
        
        records[name].name = name;
        records[name].timeInTotal += (endTime - startTime);
        records[name].runsInTotal += 1;
        auto now = std::chrono::high_resolution_clock::now();
        if ((now - records[name].printTime) > std::chrono::seconds(2)) {
            records[name].printTime = now;
            uint64_t avgTime = records[name].timeInTotal / records[name].runsInTotal;
            printf("%s: Avg GPU Time(us): %d\n", name.c_str(), int(avgTime / 1000));
        }

        glDeleteQueries(2, queries);
        return true;
    });

    for (auto it = deferredTasks.begin(); it != deferredTasks.end();) {
        if ((*it)()) {
            it = deferredTasks.erase(it);
        }
    }
}

void RunAsync(std::function<void()> task) {
    static std::queue<std::function<void()>> taskQueue;
    static std::mutex queueMutex;
    static std::condition_variable cv;
    static std::vector<std::thread> workers;
    static std::atomic<bool> stopFlag{false};
    static std::once_flag initFlag;
    static auto workerFunction = []() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, []() {
                    return !taskQueue.empty() || stopFlag.load();
                });
                if (stopFlag.load() && taskQueue.empty()) {
                    break;
                }
                if (!taskQueue.empty()) {
                    task = std::move(taskQueue.front());
                    taskQueue.pop();
                }
            } 
            if (task) {
                task();
            }
        }
    };
    
    std::call_once(initFlag, []() {
        unsigned int numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) {
            numThreads = 4;
        }
        workers.reserve(numThreads);
        for (unsigned int i = 0; i < numThreads; ++i) {
            workers.emplace_back(workerFunction);
        }
        std::atexit([]() {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                stopFlag.store(true);
            }
            cv.notify_all();
            for (auto& thread : workers) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            std::queue<std::function<void()>> empty;
            std::swap(taskQueue, empty);
        });
    });
    
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (stopFlag.load()) {
            return;
        }
        taskQueue.push(std::move(task));
    }

    cv.notify_one();
}
