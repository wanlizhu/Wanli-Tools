#pragma once
#include "Common.h"

class UnitTest_BW_UniformBuffer_GL : public Window_GL {
public:
    static constexpr int UNIFORM_BUFFER_SIZE = 4096 * 16;

    void Run(std::vector<TestResult>& results);

private:
    void Initialize();
    void Cleanup();
    void CreateBuffer();
    
private:
    uint64_t m_frameIndex = 0;
    GLuint m_program = 0;
    GLuint m_uniformBuffer = 0;
};
