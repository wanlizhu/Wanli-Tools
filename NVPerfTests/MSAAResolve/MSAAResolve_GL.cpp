#include "MSAAResolve_GLVK.h"

void MSAAResolve_GL::Run() {
    try {
        OpenWindow("MSAA Resolve GL", TEXTURE_WIDTH, TEXTURE_HEIGHT, {
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

        // Check for actual MSAA samples
        glGetIntegerv(GL_SAMPLES, &m_actualMSAASamples);
        std::cout << "Actual MSAA samples: " << m_actualMSAASamples << std::endl;
        if (m_actualMSAASamples <= 1) {
            throw std::runtime_error("No MSAA");
        }

        // Check for required extensions
        const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
        if (!strstr(extensions, "GL_ARB_framebuffer_object")) {
            throw std::runtime_error("GL_ARB_framebuffer_object extension not supported by NVIDIA driver - driver may be too old");
        }

        CreateMSAAFramebuffers();
        CreateShaderProgram();
        CreateVertexArray();
        CreateUniformBuffer();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    std::cout << "Initialization Finished" << std::endl;

    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        }

        BeginGPUTimer();
        Render();
        glFinish();
        EndGPUTimer(true);

        glfwSwapBuffers(m_window);
    }

    glDeleteProgram(m_shaderProgram);
    glDeleteVertexArrays(1, &m_vertexArray);
    glDeleteBuffers(1, &m_uniformBuffer);
    glDeleteTextures(1, &m_msaaColorTexture);
    glDeleteTextures(1, &m_resolveColorTexture);
    glDeleteFramebuffers(1, &m_msaaFramebuffer);
    glDeleteFramebuffers(1, &m_resolveFramebuffer);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void MSAAResolve_GL::CreateMSAAFramebuffers() {
    // Create MSAA framebuffer for multisampling
    glGenFramebuffers(1, &m_msaaFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFramebuffer);

    // Create MSAA color texture
    glGenTextures(1, &m_msaaColorTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_actualMSAASamples, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_TRUE);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resolveColorTexture, 0);

    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Resolve framebuffer incomplete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::cout << "MSAA framebuffer created" << std::endl;
}

GLuint CompileShader(GLenum type, const std::string& filename) {
    const char* source = nullptr;
    if (std::filesystem::exists(filename)) {
        std::cout << "Load shader source from file: " << filename << std::endl;
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << filename << std::endl;
            throw std::runtime_error("Cannot open file");
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string bufferstr = buffer.str();
        source = bufferstr.c_str();
    }
    else {
        std::cout << "Load shader source from embedded string: " << filename << std::endl;
        source = (const char*)g_files.at(filename).data();
    }

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int result = 0, length = 0;
    char* message = nullptr;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(shader, length, &length, message);
        glDeleteShader(shader);

        std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader: " << message << std::endl;
        throw std::runtime_error("Failed to compile shader");
    }

    return shader;
}

void MSAAResolve_GL::CreateShaderProgram() {
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, "vertex-shader.glsl");
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, "fragment-shader.glsl");

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    int result = 0, length = 0;
    char* message = nullptr;
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &length);
        message = (char*)alloca(length * sizeof(char));
        glGetProgramInfoLog(m_shaderProgram, length, &length, message);
        glDeleteProgram(m_shaderProgram);

        std::cerr << "Failed to link shader program: " << message << std::endl;
        throw std::runtime_error("Failed to link shader program");
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void MSAAResolve_GL::CreateVertexArray() {
    glGenVertexArrays(1, &m_vertexArray);
    glBindVertexArray(m_vertexArray);
    glBindVertexArray(0);
}

void MSAAResolve_GL::CreateUniformBuffer() {
    glGenBuffers(1, &m_uniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(unsigned int) * 4, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void MSAAResolve_GL::Render() {
    m_frameIndex++;
    glUseProgram(m_shaderProgram);

    glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFramebuffer);
    glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up uniforms
    GLint frameCountLoc = glGetUniformLocation(m_shaderProgram, "frameIndex");
    glUniform1ui(frameCountLoc, m_frameIndex);

    // Actual draw call
    glBindVertexArray(m_vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // MSAA resolve: copy from MSAA framebuffer to resolve framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolveFramebuffer);
    glBlitFramebuffer(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
        0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Copy final result to back buffer for presentation
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_resolveFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
        0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);
}