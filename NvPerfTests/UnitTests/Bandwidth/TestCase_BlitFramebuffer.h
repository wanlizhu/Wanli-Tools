#pragma once
#include "Common.h"

class UnitTest_BW_BlitFramebuffer_GL : public Window_GL {
public:
    void Run(std::vector<TestResult>& results);

private:
    void Initialize();
    void Cleanup();
    
private:
    GLuint m_blitSrcFbo = 0, m_blitDstFbo = 0;
    GLuint m_blitSrcTex = 0, m_blitDstTex = 0;
};
