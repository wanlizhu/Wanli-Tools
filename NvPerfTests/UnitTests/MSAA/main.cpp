#include "Window_GL.h"

#define INCBIN_PREFIX g_
#define INCBIN_STYLE  INCBIN_STYLE_SNAKE
#include "incbin.h"
INCTXT(vertex_shader_glsl, "vertex-shader.glsl");
INCTXT(fragment_shader_glsl, "fragment-shader.glsl");
std::unordered_map<std::string, std::vector<uint8_t>> g_embeddedFiles = {
    { "vertex-shader.glsl", std::vector<uint8_t>(g_vertex_shader_glsl_data, g_vertex_shader_glsl_data + g_vertex_shader_glsl_size) },
    { "fragment-shader.glsl", std::vector<uint8_t>(g_fragment_shader_glsl_data, g_fragment_shader_glsl_data + g_fragment_shader_glsl_size) },
};

static constexpr int _sizeof_(GLenum color) {
    switch (color) {
        case GL_RGBA8:  return 4;
        case GL_RGBA16: return 8;
        case GL_RGBA32UI: return 16;
        case GL_RGBA32F: return 16;
        default: assert(false); return 0;
    }
}

static constexpr int ChannelType(GLenum color) {
    switch (color) {
        case GL_RGBA8:  return GL_UNSIGNED_BYTE;
        case GL_RGBA16: return GL_UNSIGNED_SHORT;
        case GL_RGBA32UI: return GL_UNSIGNED_INT;
        case GL_RGBA32F: return GL_FLOAT;
        default: assert(false); return 0;
    }
}

class UnitTest_MSAA_GL : public Window_GL {
public:
    static const unsigned int TEXTURE_WIDTH = 3840;
    static const unsigned int TEXTURE_HEIGHT = 2160;
    static const unsigned int MSAA_SAMPLE_COUNT = 16;
    static const unsigned int MSAA_FORMAT = GL_RGBA8;

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
        const double sizeReadFromMSAAFB = TEXTURE_WIDTH * TEXTURE_HEIGHT * m_actualMSAASamples * _sizeof_(MSAA_FORMAT);
        const double sizeReadFromResolveFB = TEXTURE_WIDTH * TEXTURE_HEIGHT * 1 * _sizeof_(MSAA_FORMAT);
        const double sizeWriteToResolveFB = TEXTURE_WIDTH * TEXTURE_HEIGHT * 1 * _sizeof_(MSAA_FORMAT);
        const double sizeWriteToDefaultFB = TEXTURE_WIDTH * TEXTURE_HEIGHT * 1 * _sizeof_(GL_RGBA8);
        std::cout << "Actual MSAA samples: " << m_actualMSAASamples << std::endl;
        if (m_actualMSAASamples <= 1) {
            throw std::runtime_error("No MSAA");
        }

        glGenBuffers(1, &m_uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(unsigned int) * 4, nullptr, GL_DYNAMIC_DRAW);

        m_shaderProgram = CompileAndLinkShaders("vertex-shader.glsl", "fragment-shader.glsl");
        GLint frameCountLoc = glGetUniformLocation(m_shaderProgram, "frameCount");
        glUseProgram(m_shaderProgram);

        while (!glfwWindowShouldClose(m_window)) {
            glfwPollEvents();
            if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }

            BeginGPUTimer();
            DrawFullscreenTriangle();
            m_sizeReadFromMSAAFB += sizeReadFromMSAAFB;
            m_sizeReadFromResolveFB += sizeReadFromResolveFB;
            m_sizeWriteToResolveFB += sizeWriteToResolveFB;
            m_sizeWriteToDefaultFB += sizeWriteToDefaultFB;

            double fps = EndGPUTimer();
            if (fps > 0) {
                double ms = 1000.0 / fps;
                printf("Avg FPS: %.2f, Avg BW(G/s): %.2f\n", fps, 
                    (m_sizeReadFromMSAAFB + m_sizeReadFromResolveFB + m_sizeWriteToResolveFB + m_sizeWriteToDefaultFB) / ms / (1024 * 1024 * 1024));
                
                m_sizeReadFromMSAAFB = 0.0;
                m_sizeReadFromResolveFB = 0.0;
                m_sizeWriteToResolveFB = 0.0;
                m_sizeWriteToDefaultFB = 0.0;
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
    void CreateMSAAFramebuffers() {
        // Create MSAA framebuffer for multisampling
        glGenFramebuffers(1, &m_msaaFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFramebuffer);

        // Create MSAA color texture
        glGenTextures(1, &m_msaaColorTexture);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_actualMSAASamples, MSAA_FORMAT, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture, 0);

        // Check framebuffer status
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("MSAA framebuffer incomplete");
        }

        // Create resolve framebuffer (always needed for final output)
        glGenFramebuffers(1, &m_resolveFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_resolveFramebuffer);

        // Create resolve color texture
        glGenTextures(1, &m_resolveColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_resolveColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, MSAA_FORMAT, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, ChannelType(MSAA_FORMAT), nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resolveColorTexture, 0);

        // Check framebuffer status
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Resolve framebuffer incomplete");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        std::cout << "MSAA framebuffer created with " << m_actualMSAASamples << " samples" << std::endl;
    }

    void DrawFullscreenTriangle() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFramebuffer);
        glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        glUniform1ui(m_frameCountLoc, m_frameCount++);

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

        // This test is GPU bound
        glFinish();
    }

private:
    GLuint m_msaaFramebuffer = 0;
    GLuint m_msaaColorTexture = 0;
    GLuint m_resolveFramebuffer = 0;
    GLuint m_resolveColorTexture = 0;
    GLuint m_shaderProgram = 0;
    GLuint m_uniformBuffer = 0;
    GLuint m_frameCountLoc = 0;
    int m_frameCount = 0;
    int m_actualMSAASamples = 0;

    double m_sizeReadFromMSAAFB = 0.0;
    double m_sizeReadFromResolveFB = 0.0;
    double m_sizeWriteToResolveFB = 0.0;
    double m_sizeWriteToDefaultFB = 0.0;
};

int main(int argc, char** argv) {
    UnitTest_MSAA_GL test;
    test.Run();
    return 0;
}