#include "TestCase_BlitFramebuffer.h"

void UnitTest_BW_BlitFramebuffer_GL::Run(std::vector<TestResult>& results) {
    std::cout << std::endl;

    OpenWindow("Bandwidth Test - BlitFramebuffer", TEXTURE_WIDTH, TEXTURE_HEIGHT, {
        {GLFW_CONTEXT_VERSION_MAJOR, 4},
        {GLFW_CONTEXT_VERSION_MINOR, 3},
        {GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE}
    });
    
    Initialize();
    
    for (int warmup = 0; warmup < 100; warmup++) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_blitSrcFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_blitDstFbo);
        glBlitFramebuffer(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_blitDstFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(100, 100, 101, 101, 100, 100, 101, 101, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glfwSwapBuffers(m_window);
    }
    
    ReadAndResetTimer();
    double frames = 0.0;
    double totalTime = 0.0;

    while (ReadTimer() < 2000) {
        BeginGPUTimer();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_blitSrcFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_blitDstFbo);
        glBlitFramebuffer(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_blitDstFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(100, 100, 101, 101, 100, 100, 101, 101, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glFinish();
        totalTime += EndGPUTimer(true);
        frames += 1;

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    
    double fps = 1000.0 / (totalTime / frames);
    double bandwidth = (TEXTURE_WIDTH * TEXTURE_HEIGHT * 4.0 * fps) / 1e9;
    
    Cleanup();
    CloseWindow();
    
    results.push_back({"glBlitFramebuffer", "Read", bandwidth, fps});
}

void UnitTest_BW_BlitFramebuffer_GL::Initialize() {
    glGenFramebuffers(1, &m_blitSrcFbo);
    glGenTextures(1, &m_blitSrcTex);
    glBindTexture(GL_TEXTURE_2D, m_blitSrcTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, m_blitSrcFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blitSrcTex, 0);

    glGenFramebuffers(1, &m_blitDstFbo);
    glGenTextures(1, &m_blitDstTex);
    glBindTexture(GL_TEXTURE_2D, m_blitDstTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, m_blitDstFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blitDstTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UnitTest_BW_BlitFramebuffer_GL::Cleanup() {
    glDeleteFramebuffers(1, &m_blitSrcFbo);
    glDeleteFramebuffers(1, &m_blitDstFbo);
    glDeleteTextures(1, &m_blitSrcTex);
    glDeleteTextures(1, &m_blitDstTex);
}
