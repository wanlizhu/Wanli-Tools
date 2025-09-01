#include "HookUtils.h"

std::unordered_map<std::string, void*> replacements;

__attribute__((constructor))
static void GLHooks_BEGIN() {
    replacements["glBlitFramebuffer"] = (void*)glBlitFramebuffer;
}

__attribute__((destructor))
static void GLHooks_END() {
}

extern "C" void (*glXGetProcAddress(const GLubyte *procname))(void) {
    static auto real_glXGetProcAddress = (decltype(&glXGetProcAddress))dlsym(RTLD_NEXT, "glXGetProcAddress");

    auto it = replacements.find((const char*)procname);
    if (it != replacements.end()) {
        return (void (*)(void))it->second;
    }

    return real_glXGetProcAddress(procname);
}

extern "C" void (*glXGetProcAddressARB(const GLubyte *procname))(void) {
    static auto real_glXGetProcAddressARB = (decltype(&glXGetProcAddressARB))dlsym(RTLD_NEXT, "glXGetProcAddressARB");
    
    auto it = replacements.find((const char*)procname);
    if (it != replacements.end()) {
        return (void (*)(void))it->second;
    }

    return real_glXGetProcAddressARB(procname);
}

void RunWithGPUTimer(
    const std::string& name,
    const std::function<void()>& func
) {
    static auto glGenQueries = (PFNGLGENQUERIESPROC)dlsym(RTLD_NEXT, "glGenQueries");
    static auto glQueryCounter = (PFNGLQUERYCOUNTERPROC)dlsym(RTLD_NEXT, "glQueryCounter");
    static auto glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC)dlsym(RTLD_NEXT, "glGetQueryObjectiv");
    static auto glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC)dlsym(RTLD_NEXT, "glGetQueryObjectui64v");
    static auto glDeleteQueries = (PFNGLDELETEQUERIESPROC)dlsym(RTLD_NEXT, "glDeleteQueries");
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
