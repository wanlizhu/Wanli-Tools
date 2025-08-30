#pragma once
#include "Common.h"

class UnitTest_BW_Cubemap_GL : public Window_GL {
public:
    static constexpr int CUBE_SIZE = 1024;

    void Run(std::vector<TestResult>& results);

private:
    void Initialize();
    void Cleanup();
    void CreateTextures();
    TestResult TestTexture(GLuint texture, const std::string& format);
    
private:
    uint64_t m_frameIndex = 0;
    GLuint m_program = 0;
    GLuint m_cubemap_RGB8 = 0, m_cubemap_RGBA16 = 0;
};
