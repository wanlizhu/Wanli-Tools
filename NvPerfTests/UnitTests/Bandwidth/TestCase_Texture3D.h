#pragma once
#include "Common.h"

class UnitTest_BW_Texture3D_GL : public Window_GL {
public:
    static constexpr int TEX3D_SIZE = 256;

    void Run(std::vector<TestResult>& results);

private:
    void Initialize();
    void Cleanup();
    void CreateTextures();
    TestResult TestTexture(GLuint texture, const std::string& format);
    
private:
    uint64_t m_frameIndex = 0;
    GLuint m_program = 0;
    GLuint m_texture_RGB8 = 0, m_texture_RGBA16 = 0;
};
