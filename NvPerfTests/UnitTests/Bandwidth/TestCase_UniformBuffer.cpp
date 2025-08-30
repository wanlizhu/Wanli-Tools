#include "TestCase_UniformBuffer.h"

void UnitTest_BW_UniformBuffer_GL::Run(std::vector<TestResult>& results) {
    std::cout << std::endl;

    OpenWindow("Bandwidth Test - UniformBuffer", TEXTURE_WIDTH, TEXTURE_HEIGHT, {
        {GLFW_CONTEXT_VERSION_MAJOR, 4},
        {GLFW_CONTEXT_VERSION_MINOR, 3},
        {GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE}
    });
    
    Initialize();
    
    for (int warmup = 0; warmup < 100; warmup++) {
        glUseProgram(m_program);
        glUniform1ui(glGetUniformLocation(m_program, "frameIndex"), m_frameIndex++);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniformBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    
    double frames = 0;
    double timeInTotal = 0.0;
    ReadAndResetTimer();
    
    while (ReadTimer() < 2000) {
        BeginGPUTimer();
        
        glUseProgram(m_program);
        glUniform1ui(glGetUniformLocation(m_program, "frameIndex"), m_frameIndex++);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniformBuffer);
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
    
    Cleanup();
    CloseWindow();
    
    results.push_back({"Uniform Buffer", "Read", bandwidth, fps});
}

void UnitTest_BW_UniformBuffer_GL::Initialize() {
    m_program = CompileAndLinkShaders("Common.vert.glsl", "TestCase_UniformBuffer.frag.glsl");
    CreateBuffer();
}

void UnitTest_BW_UniformBuffer_GL::CreateBuffer() {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    glGenBuffers(1, &m_uniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
    std::vector<float> uniformData(UNIFORM_BUFFER_SIZE / sizeof(float));
    for (auto& v : uniformData) v = dist(rng);
    glBufferData(GL_UNIFORM_BUFFER, UNIFORM_BUFFER_SIZE, uniformData.data(), GL_DYNAMIC_DRAW);
}

void UnitTest_BW_UniformBuffer_GL::Cleanup() {
    glDeleteProgram(m_program);
    glDeleteBuffers(1, &m_uniformBuffer);
}
