#include "Window_GL.h"
#include <chrono>
#include <cstdint>
#include <ostream>

#define INCBIN_PREFIX g_
#define INCBIN_STYLE  INCBIN_STYLE_SNAKE
#include "incbin.h"
INCTXT(vertex_shader_glsl, "vertex-shader.glsl");
INCTXT(fragment_shader_glsl, "fragment-shader.glsl");
std::unordered_map<std::string, std::vector<uint8_t>> g_embeddedFiles = {
    { "vertex-shader.glsl", std::vector<uint8_t>(g_vertex_shader_glsl_data, g_vertex_shader_glsl_data + g_vertex_shader_glsl_size) },
    { "fragment-shader.glsl", std::vector<uint8_t>(g_fragment_shader_glsl_data, g_fragment_shader_glsl_data + g_fragment_shader_glsl_size) },
};

class UnitTest_MSAA_GL : public Window_GL {
public:
    static const unsigned int TEXTURE_WIDTH = 3840;
    static const unsigned int TEXTURE_HEIGHT = 2160;
    static const unsigned int MSAA_SAMPLE_COUNT = 16;
    static const unsigned int MSAA_FORMAT = GL_RGBA32F;

    // Resolve blit: DRAM -> L2(decompress) -> ROP resolve -> L2(compress) -> DRAM 
    // Present blit: DRAM -> L2 -> ROP copy -> L2 -> DRAM  
    // You don’t see L2 decompression on the present copy if the source is L2-resident or already uncompressed in memory
    // You don’t see compression if the default framebuffer is allocated or treated as non-compressible for display/compositor reasons.
    void Run() {
        OpenWindow("GL Unit Test: MSAA Resolve", TEXTURE_WIDTH, TEXTURE_HEIGHT, {
            {GLFW_CONTEXT_VERSION_MAJOR, 4},
            {GLFW_CONTEXT_VERSION_MINOR, 5},
            {GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE},
            {GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE},
            {GLFW_SAMPLES, MSAA_SAMPLE_COUNT},
            {GLFW_RED_BITS, 8},
            {GLFW_GREEN_BITS, 8},
            {GLFW_BLUE_BITS, 8},
            {GLFW_ALPHA_BITS, 8},
            {GLFW_DEPTH_BITS, 24},
            {GLFW_STENCIL_BITS, 8},
            {GLFW_DOUBLEBUFFER, GLFW_TRUE},
        });

        glGetIntegerv(GL_SAMPLES, &m_actualMSAASamples);
        uint64_t sizeReadFromMSAAFB = TEXTURE_WIDTH * TEXTURE_HEIGHT * m_actualMSAASamples * _sizeof_(MSAA_FORMAT);
        uint64_t sizeReadFromResolveFB = TEXTURE_WIDTH * TEXTURE_HEIGHT * 1 * _sizeof_(MSAA_FORMAT);
        uint64_t sizeWriteToResolveFB = TEXTURE_WIDTH * TEXTURE_HEIGHT * 1 * _sizeof_(MSAA_FORMAT);
        uint64_t sizeWriteToDefaultFB = TEXTURE_WIDTH * TEXTURE_HEIGHT * 1 * _sizeof_(GL_RGBA8);
        uint64_t sizeReadWritePerFrame = sizeReadFromMSAAFB + sizeReadFromResolveFB + sizeWriteToResolveFB + sizeWriteToDefaultFB;

        std::cout << "Actual MSAA samples: " << m_actualMSAASamples << std::endl;
        std::cout << "Actual MSAA color format: " << _str_(MSAA_FORMAT) << std::endl;
        std::cout << "Actual MSAA texture size: " << TEXTURE_WIDTH << "x" << TEXTURE_HEIGHT << std::endl;
        std::cout << "Actual size read & write per frame: " << _size_(sizeReadWritePerFrame) << std::endl;
        if (m_actualMSAASamples <= 1) {
            throw std::runtime_error("No MSAA");
        }

        glGenBuffers(1, &m_uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(unsigned int) * 4, nullptr, GL_DYNAMIC_DRAW);
        CreateMSAAFramebuffers();

        m_shaderProgram = CompileAndLinkShaders("vertex-shader.glsl", "fragment-shader.glsl");
        GLint frameCountLoc = glGetUniformLocation(m_shaderProgram, "frameIndex");
        glUseProgram(m_shaderProgram);

        double timeNs = 0.0;
        double frames = 0;
        double sizeReadWrite = 0;
        ResetCPUTimer();

        while (!glfwWindowShouldClose(m_window)) {
            glfwPollEvents();
            if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }

            BeginGPUTimer();
            DrawFullscreenTriangle();
            EndGPUTimer();
            glFinish();

            timeNs += ReadGPUTimer().count();
            frames += 1;
            sizeReadWrite += sizeReadWritePerFrame;

            if (ReadCPUTimer() > std::chrono::seconds(3)) {
                printf("Avg FPS: %.2f, Avg BW(GB/s): %.2f\n", 1000.0 / (timeNs / 1e6 / frames), 
                    sizeReadWrite / (timeNs / 1e6 / frames) / (1024 * 1024 * 1024));
                frames = 0;
                timeNs = 0;
                sizeReadWrite = 0;
                ResetCPUTimer();
            }

            glfwSwapBuffers(m_window);
        }

        glDeleteProgram(m_shaderProgram);
        glDeleteBuffers(1, &m_uniformBuffer);
        glDeleteTextures(1, &m_msaaColorTexture);
        glDeleteTextures(1, &m_resolveColorTexture);
        glDeleteFramebuffers(1, &m_msaaFramebuffer);
        glDeleteFramebuffers(1, &m_resolveFramebuffer);
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

private:
    void ValidateFramebuffer() {
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            const char* errorMsg = nullptr;
            switch (status) {
                case GL_FRAMEBUFFER_UNDEFINED:
                    errorMsg = "GL_FRAMEBUFFER_UNDEFINED: The specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: One or more framebuffer attachment points are incomplete";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: The framebuffer does not have at least one image attached to it";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: The value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for one or more color attachment points named by GL_DRAW_BUFFERi";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER";
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    errorMsg = "GL_FRAMEBUFFER_UNSUPPORTED: The combination of internal formats of the attached images violates an implementation-dependent set of restrictions";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: The value of GL_RENDERBUFFER_SAMPLES or GL_TEXTURE_SAMPLES is not the same for all attached images, or the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: A framebuffer attachment is layered, and another is not layered, or all attachments are layered but not all attachments are from textures of the same target";
                    break;
                default:
                    errorMsg = "Unknown framebuffer error";
                    break;
            }
            std::cerr << "MSAA framebuffer incomplete: " << errorMsg << " (Status: 0x" << std::hex << status << std::dec << ")" << std::endl;
            throw std::runtime_error(errorMsg);
        }
    }

    void CreateMSAAFramebuffers() {
        // Create MSAA framebuffer for multisampling
        glGenFramebuffers(1, &m_msaaFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFramebuffer);

        // Create MSAA color texture
        glGenTextures(1, &m_msaaColorTexture);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_actualMSAASamples, MSAA_FORMAT, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture, 0);
        ValidateFramebuffer();

        // Create resolve framebuffer (always needed for final output)
        glGenFramebuffers(1, &m_resolveFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_resolveFramebuffer);

        // Create resolve color texture
        glGenTextures(1, &m_resolveColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_resolveColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, MSAA_FORMAT, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, _type_(MSAA_FORMAT), nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resolveColorTexture, 0);
        ValidateFramebuffer();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        std::cout << "MSAA framebuffer created with " << m_actualMSAASamples << " samples" << std::endl;
    }

    void DrawFullscreenTriangle() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFramebuffer);
        glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        glUniform1ui(m_frameIndexLoc, m_frameIndex++);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // MSAA resolve: copy from MSAA framebuffer to resolve framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolveFramebuffer);
        glBlitFramebuffer(
            0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
            0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
            GL_COLOR_BUFFER_BIT, GL_LINEAR
        );

        // Copy final result to back buffer for presentation
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_resolveFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
            0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
            GL_COLOR_BUFFER_BIT, GL_LINEAR
        );
    }

private:
    GLuint m_msaaFramebuffer = UINT32_MAX;
    GLuint m_msaaColorTexture = UINT32_MAX;
    GLuint m_resolveFramebuffer = UINT32_MAX;
    GLuint m_resolveColorTexture = UINT32_MAX;
    GLuint m_shaderProgram = UINT32_MAX;
    GLuint m_uniformBuffer = UINT32_MAX;
    GLuint m_frameIndexLoc = UINT32_MAX;
    int m_frameIndex = 0;
    int m_actualMSAASamples = 0;
};

int main(int argc, char** argv) {
    UnitTest_MSAA_GL test;
    test.Run();
    return 0;
}