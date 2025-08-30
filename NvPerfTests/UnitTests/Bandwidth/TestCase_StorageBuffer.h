#pragma once
#include "Common.h"

class UnitTest_BW_StorageBuffer_GL : public Window_GL {
public:
    static constexpr int STORAGE_BUFFER_SIZE = 16 * 1024 * 1024;

    void Run(std::vector<TestResult>& results);

private:
    void Initialize();
    void Cleanup();
    void CreateBuffer();
    TestResult TestRead();
    TestResult TestWrite();
    
private:
    uint64_t m_frameIndex = 0;
    GLuint m_programRead = 0;
    GLuint m_programWrite = 0;
    GLuint m_storageBuffer = 0;
};