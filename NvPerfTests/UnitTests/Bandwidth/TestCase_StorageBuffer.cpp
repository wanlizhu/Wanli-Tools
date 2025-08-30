#include "TestCase_StorageBuffer.h"

void UnitTest_BW_StorageBuffer_GL::Run(std::vector<TestResult>& results) {
    std::cout << std::endl;

    OpenWindow("Bandwidth Test - StorageBuffer", TEXTURE_WIDTH, TEXTURE_HEIGHT, {
        {GLFW_CONTEXT_VERSION_MAJOR, 4},
        {GLFW_CONTEXT_VERSION_MINOR, 3},
        {GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE}
    });
    
    Initialize();
    
    results.push_back(TestRead());
    results.push_back(TestWrite());
    
    Cleanup();
    CloseWindow();
}

TestResult UnitTest_BW_StorageBuffer_GL::TestRead() {
    for (int warmup = 0; warmup < 100; warmup++) {
        glUseProgram(m_programRead);
        glUniform1ui(glGetUniformLocation(m_programRead, "frameIndex"), m_frameIndex++);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_storageBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    
    double frames = 0;
    double timeInTotal = 0.0;
    ReadAndResetTimer();
    
    while (ReadTimer() < 2000) {
        BeginGPUTimer();
        
        glUseProgram(m_programRead);
        glUniform1ui(glGetUniformLocation(m_programRead, "frameIndex"), m_frameIndex++);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_storageBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glFinish();
        timeInTotal += EndGPUTimer(true);
        frames += 1;

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    
    double fps = 1000.0 / (timeInTotal / frames);
    double ops_per_pixel = 64.0;
    double pixels_per_frame = TEXTURE_WIDTH * TEXTURE_HEIGHT;
    double bandwidth = (pixels_per_frame * ops_per_pixel * 16.0 * fps) / 1e9;
    
    return {"Storage Buffer", "Read", bandwidth, fps};
}

TestResult UnitTest_BW_StorageBuffer_GL::TestWrite() {
    for (int warmup = 0; warmup < 100; warmup++) {
        glUseProgram(m_programWrite);
        glUniform1ui(glGetUniformLocation(m_programWrite, "frameIndex"), m_frameIndex++);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_storageBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    
    double frames = 0;
    double timeInTotal = 0.0;
    ReadAndResetTimer();
    
    while (ReadTimer() < 2000) {
        BeginGPUTimer();
        
        glUseProgram(m_programWrite);
        glUniform1ui(glGetUniformLocation(m_programWrite, "frameIndex"), m_frameIndex++);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_storageBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glFinish();
        timeInTotal += EndGPUTimer(true);
        frames += 1;

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    
    double fps = 1000.0 / (timeInTotal / frames);
    double ops_per_pixel = 64.0;
    double pixels_per_frame = TEXTURE_WIDTH * TEXTURE_HEIGHT;
    double bandwidth = (pixels_per_frame * ops_per_pixel * 16.0 * fps) / 1e9;
    
    return {"Storage Buffer", "Write", bandwidth, fps};
}

void UnitTest_BW_StorageBuffer_GL::Initialize() {
    m_programRead = CompileAndLinkShaders("Common.vert.glsl", "TestCase_StorageBuffer_read.frag.glsl");
    m_programWrite = CompileAndLinkShaders("Common.vert.glsl", "TestCase_StorageBuffer_write.frag.glsl");
    CreateBuffer();
}

void UnitTest_BW_StorageBuffer_GL::CreateBuffer() {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    glGenBuffers(1, &m_storageBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_storageBuffer);
    std::vector<float> storageData(STORAGE_BUFFER_SIZE / sizeof(float));
    for (auto& v : storageData) v = dist(rng);
    glBufferData(GL_SHADER_STORAGE_BUFFER, STORAGE_BUFFER_SIZE, storageData.data(), GL_DYNAMIC_COPY);
}

void UnitTest_BW_StorageBuffer_GL::Cleanup() {
    glDeleteProgram(m_programRead);
    glDeleteProgram(m_programWrite);
    glDeleteBuffers(1, &m_storageBuffer);
}
