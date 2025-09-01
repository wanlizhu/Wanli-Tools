#include "HookUtils.h"

// Weak glibc entry so we don't hardcode version strings; absent on non-glibc.
extern "C" void* __libc_dlsym(void* handle, const char* symbol) __attribute__((weak));

__attribute__((constructor))
static void GLHooks_BEGIN() {
    if (!__libc_dlsym) {
        fprintf(stderr, "Warning: __libc_dlsym not available\n");
    }
}

__attribute__((destructor))
static void GLHooks_END() {
}

void* LoadRealAPI(const char* name) {
    using PFN_glXGetProcAddress = void* (*)(const GLubyte*);
    using PFN_eglGetProcAddress = void* (*)(const char*);

    static bool inited = false;
    static PFN_glXGetProcAddress real_glXGetProcAddress = nullptr;
    static PFN_glXGetProcAddress real_glXGetProcAddressARB = nullptr;
    static PFN_eglGetProcAddress real_eglGetProcAddress = nullptr;
    if (inited == false && __libc_dlsym) {
        inited = true;
        real_glXGetProcAddress = (PFN_glXGetProcAddress) __libc_dlsym(RTLD_NEXT, "glXGetProcAddress");
        real_glXGetProcAddressARB = (PFN_glXGetProcAddress) __libc_dlsym(RTLD_NEXT, "glXGetProcAddressARB");
        real_eglGetProcAddress = (PFN_eglGetProcAddress) __libc_dlsym(RTLD_NEXT, "eglGetProcAddress");
    }
    
    void* addr = nullptr;
    if (__libc_dlsym) {
        addr = __libc_dlsym(RTLD_NEXT, name);
    }
    if (addr == nullptr && real_glXGetProcAddress) {
        addr = real_glXGetProcAddress((const GLubyte*)name);
    }
    if (addr == nullptr && real_glXGetProcAddressARB) {
        addr = real_glXGetProcAddressARB((const GLubyte*)name);
    }
    if (addr == nullptr && real_eglGetProcAddress) {
        addr = real_eglGetProcAddress(name);
    }

    return addr;
}

void RunWithGPUTimer(
    const std::string& name,
    const std::function<void()>& func
) {
    static auto glGenQueries = (PFNGLGENQUERIESPROC)LoadRealAPI("glGenQueries");
    static auto glQueryCounter = (PFNGLQUERYCOUNTERPROC)LoadRealAPI("glQueryCounter");
    static auto glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC)LoadRealAPI("glGetQueryObjectiv");
    static auto glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC)LoadRealAPI("glGetQueryObjectui64v");
    static auto glDeleteQueries = (PFNGLDELETEQUERIESPROC)LoadRealAPI("glDeleteQueries");
    static std::unordered_map<std::string, GPUTimerRec> records;
    static std::mutex recordsMutex;

    GLuint queries[2];
    glGenQueries(2, queries);

    glQueryCounter(queries[0], GL_TIMESTAMP);
    func();
    glQueryCounter(queries[1], GL_TIMESTAMP);

    RunAsync([&, name, queries](){
        GLint a0, a1;
        do {
            glGetQueryObjectiv(queries[0], GL_QUERY_RESULT_AVAILABLE, &a0);
            glGetQueryObjectiv(queries[1], GL_QUERY_RESULT_AVAILABLE, &a1);
            usleep(100);
        } while (!(a0 && a1));

        GLuint64 startTime = 0, endTime = 0;
        glGetQueryObjectui64v(queries[0], GL_QUERY_RESULT, &startTime);
        glGetQueryObjectui64v(queries[1], GL_QUERY_RESULT, &endTime);
        
        {
            std::unique_lock<std::mutex> lock(recordsMutex);
            records[name].name = name;
            records[name].timeInTotal += (endTime - startTime);
            records[name].runsInTotal += 1;
            auto now = std::chrono::high_resolution_clock::now();
            if ((now - records[name].printTime) > std::chrono::seconds(2)) {
                records[name].printTime = now;
                printf("%s(...): Avg GPU Time(us): %d\n", name.c_str(), int((endTime - startTime) / 1000));
            }
        }

        glDeleteQueries(2, queries);
    });
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
