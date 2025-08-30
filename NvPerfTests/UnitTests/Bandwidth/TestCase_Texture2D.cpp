#include "TestCase_Texture2D.h"
#include "Window_GL.h"

void UnitTest_BW_Texture2D_GL::Run(std::vector<TestResult>& results) {
    std::cout << std::endl;

    OpenWindow("Bandwidth Test - Texture2D", TEXTURE_WIDTH, TEXTURE_HEIGHT, {
        {GLFW_CONTEXT_VERSION_MAJOR, 4},
        {GLFW_CONTEXT_VERSION_MINOR, 3},
        {GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE}
    });
    
    Initialize();
    
    results.push_back(TestTexture(m_texture_RGB8, "RGB8"));
    results.push_back(TestTexture(m_texture_RGBA16, "RGBA16"));
    results.push_back(TestTexture(m_texture_DXT1, "DXT1 (Compressed)"));
    results.push_back(TestTexture(m_texture_DXT5, "DXT5 (Compressed)"));
    
    Cleanup();
    CloseWindow();
}

TestResult UnitTest_BW_Texture2D_GL::TestTexture(GLuint texture, const std::string& format) {
    for (int warmup = 0; warmup < 100; warmup++) {
        glUseProgram(m_program);
        glUniform1ui(glGetUniformLocation(m_program, "frameIndex"), m_frameIndex++);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(m_program, "tex2D"), 0);
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
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(m_program, "tex2D"), 0);
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
    
    double bytes_per_pixel = 0;
    if (format == "RGB8") bytes_per_pixel = 3.0;
    else if (format == "RGBA16") bytes_per_pixel = 8.0;
    else if (format == "DXT1 (Compressed)") bytes_per_pixel = 0.5;
    else if (format == "DXT5 (Compressed)") bytes_per_pixel = 1.0;
    
    double bandwidth = (pixels_per_frame * ops_per_pixel * bytes_per_pixel * fps) / 1e9;
    
    return {"Texture2D " + format, "Read", bandwidth, fps};
}

void UnitTest_BW_Texture2D_GL::Initialize() {
    m_program = CompileAndLinkShaders("Common.vert.glsl", "TestCase_Texture2D.frag.glsl");
    CreateTextures();
}

void UnitTest_BW_Texture2D_GL::CreateTextures() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<uint8_t> dist(0, 255);

    glGenTextures(1, &m_texture_RGB8);
    glBindTexture(GL_TEXTURE_2D, m_texture_RGB8);
    std::vector<uint8_t> data_RGB8(TEX_SIZE * TEX_SIZE * 3);
    for (auto& v : data_RGB8) v = dist(rng);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, data_RGB8.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &m_texture_RGBA16);
    glBindTexture(GL_TEXTURE_2D, m_texture_RGBA16);
    std::vector<uint16_t> data_RGBA16(TEX_SIZE * TEX_SIZE * 4);
    for (auto& v : data_RGBA16) v = dist(rng) * 256 + dist(rng);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, GL_UNSIGNED_SHORT, data_RGBA16.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &m_texture_DXT1);
    glBindTexture(GL_TEXTURE_2D, m_texture_DXT1);
    size_t dxt1Size = ((TEX_SIZE + 3) / 4) * ((TEX_SIZE + 3) / 4) * 8;
    std::vector<uint8_t> data_DXT1(dxt1Size);
    for (auto& v : data_DXT1) v = dist(rng);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, TEX_SIZE, TEX_SIZE, 0, dxt1Size, data_DXT1.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &m_texture_DXT5);
    glBindTexture(GL_TEXTURE_2D, m_texture_DXT5);
    size_t dxt5Size = ((TEX_SIZE + 3) / 4) * ((TEX_SIZE + 3) / 4) * 16;
    std::vector<uint8_t> data_DXT5(dxt5Size);
    for (auto& v : data_DXT5) v = dist(rng);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, TEX_SIZE, TEX_SIZE, 0, dxt5Size, data_DXT5.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void UnitTest_BW_Texture2D_GL::Cleanup() {
    glDeleteProgram(m_program);
    glDeleteTextures(1, &m_texture_RGB8);
    glDeleteTextures(1, &m_texture_RGBA16);
    glDeleteTextures(1, &m_texture_DXT1);
    glDeleteTextures(1, &m_texture_DXT5);
}
