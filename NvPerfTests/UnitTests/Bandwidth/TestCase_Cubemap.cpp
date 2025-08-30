#include "TestCase_Cubemap.h"

void UnitTest_BW_Cubemap_GL::Run(std::vector<TestResult>& results) {
    std::cout << std::endl;

    OpenWindow("Bandwidth Test - Cubemap", TEXTURE_WIDTH, TEXTURE_HEIGHT, {
        {GLFW_CONTEXT_VERSION_MAJOR, 4},
        {GLFW_CONTEXT_VERSION_MINOR, 3},
        {GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE}
    });
    
    Initialize();
    
    results.push_back(TestTexture(m_cubemap_RGB8, "RGB8"));
    results.push_back(TestTexture(m_cubemap_RGBA16, "RGBA16"));
    
    Cleanup();
    CloseWindow();
}

TestResult UnitTest_BW_Cubemap_GL::TestTexture(GLuint texture, const std::string& format) {
    for (int warmup = 0; warmup < 100; warmup++) {
        glUseProgram(m_program);
        glUniform1ui(glGetUniformLocation(m_program, "frameIndex"), m_frameIndex++);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        glUniform1i(glGetUniformLocation(m_program, "texCube"), 0);
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
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        glUniform1i(glGetUniformLocation(m_program, "texCube"), 0);
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
    double bytes_per_pixel = (format == "RGB8") ? 3.0 : 8.0;
    double bandwidth = (pixels_per_frame * ops_per_pixel * bytes_per_pixel * fps) / 1e9;
    
    return {"Cubemap " + format, "Read", bandwidth, fps};
}

void UnitTest_BW_Cubemap_GL::Initialize() {
    m_program = CompileAndLinkShaders("Common.vert.glsl", "TestCase_Cubemap.frag.glsl");
    CreateTextures();
}

void UnitTest_BW_Cubemap_GL::CreateTextures() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<uint8_t> dist(0, 255);

    glGenTextures(1, &m_cubemap_RGB8);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_RGB8);
    std::vector<uint8_t> data_RGB8(CUBE_SIZE * CUBE_SIZE * 3);
    for (int face = 0; face < 6; face++) {
        for (auto& v : data_RGB8) v = dist(rng);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB8, CUBE_SIZE, CUBE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, data_RGB8.data());
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &m_cubemap_RGBA16);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_RGBA16);
    std::vector<uint16_t> data_RGBA16(CUBE_SIZE * CUBE_SIZE * 4);
    for (int face = 0; face < 6; face++) {
        for (auto& v : data_RGBA16) v = dist(rng) * 256 + dist(rng);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA16, CUBE_SIZE, CUBE_SIZE, 0, GL_RGBA, GL_UNSIGNED_SHORT, data_RGBA16.data());
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void UnitTest_BW_Cubemap_GL::Cleanup() {
    glDeleteProgram(m_program);
    glDeleteTextures(1, &m_cubemap_RGB8);
    glDeleteTextures(1, &m_cubemap_RGBA16);
}
