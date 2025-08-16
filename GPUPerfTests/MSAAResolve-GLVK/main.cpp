#include "glad.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

class OpenGLMSAAResolve {
private:
    static const unsigned int TEXTURE_WIDTH = 3840;
    static const unsigned int TEXTURE_HEIGHT = 2160;
    static const unsigned int MSAA_SAMPLE_COUNT = 8;

    // X11 variables
    Display* m_display;
    Window m_window;
    GLXContext m_context;
    Colormap m_colormap;
    
    // OpenGL objects
    unsigned int m_msaaFramebuffer;
    unsigned int m_msaaColorTexture;
    unsigned int m_resolveFramebuffer;
    unsigned int m_resolveColorTexture;
    unsigned int m_shaderProgram;
    unsigned int m_vertexArray;
    unsigned int m_uniformBuffer;
    
    unsigned int m_frameCount;

public:
    OpenGLMSAAResolve() : 
        m_display(nullptr), m_window(0), m_context(nullptr), m_colormap(0),
        m_msaaFramebuffer(0), m_msaaColorTexture(0), m_resolveFramebuffer(0), 
        m_resolveColorTexture(0), m_shaderProgram(0), m_vertexArray(0), 
        m_uniformBuffer(0), m_frameCount(0) {}

    ~OpenGLMSAAResolve() {
        Cleanup();
    }

    bool Initialize() {
        if (!InitializeX11()) {
            std::cerr << "Failed to initialize X11" << std::endl;
            return false;
        }

        if (!InitializeOpenGL()) {
            std::cerr << "Failed to initialize OpenGL" << std::endl;
            return false;
        }

        if (!CreateMSAAFramebuffers()) {
            std::cerr << "Failed to create MSAA framebuffers" << std::endl;
            return false;
        }

        if (!LoadShaders()) {
            std::cerr << "Failed to load shaders" << std::endl;
            return false;
        }

        if (!CreateVertexArray()) {
            std::cerr << "Failed to create vertex array" << std::endl;
            return false;
        }

        if (!CreateUniformBuffer()) {
            std::cerr << "Failed to create uniform buffer" << std::endl;
            return false;
        }

        return true;
    }

    void Run() {
        XEvent event;
        bool running = true;

        while (running) {
            while (XPending(m_display) > 0) {
                XNextEvent(m_display, &event);
                switch (event.type) {
                    case ClientMessage:
                        if (event.xclient.data.l[0] == XInternAtom(m_display, "WM_DELETE_WINDOW", False)) {
                            running = false;
                        }
                        break;
                    case KeyPress:
                        if (XLookupKeysym(&event.xkey, 0) == XK_Escape) {
                            running = false;
                        }
                        break;
                }
            }

            if (running) {
                Render();
                glXSwapBuffers(m_display, m_window);
            }
        }
    }

private:
    bool InitializeX11() {
        // Open display
        m_display = XOpenDisplay(nullptr);
        if (!m_display) {
            std::cerr << "Cannot open display" << std::endl;
            return false;
        }

        // Check for GLX extension
        int glxMajor, glxMinor;
        if (!glXQueryVersion(m_display, &glxMajor, &glxMinor)) {
            std::cerr << "GLX extension not supported" << std::endl;
            return false;
        }

        std::cout << "GLX version: " << glxMajor << "." << glxMinor << std::endl;

        // Choose framebuffer config with MSAA support
        int visualAttribs[] = {
            GLX_X_RENDERABLE    , True,
            GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
            GLX_RENDER_TYPE     , GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
            GLX_RED_SIZE        , 8,
            GLX_GREEN_SIZE      , 8,
            GLX_BLUE_SIZE       , 8,
            GLX_ALPHA_SIZE      , 8,
            GLX_DEPTH_SIZE      , 24,
            GLX_STENCIL_SIZE    , 8,
            GLX_DOUBLEBUFFER    , True,
            GLX_SAMPLE_BUFFERS  , 1,
            GLX_SAMPLES         , MSAA_SAMPLE_COUNT,
            None
        };

        int fbCount;
        GLXFBConfig* fbConfigs = glXChooseFBConfig(m_display, DefaultScreen(m_display), visualAttribs, &fbCount);
        if (!fbConfigs || fbCount == 0) {
            std::cerr << "No suitable framebuffer config found with MSAA" << std::endl;
            return false;
        }

        GLXFBConfig fbConfig = fbConfigs[0];
        XVisualInfo* visualInfo = glXGetVisualFromFBConfig(m_display, fbConfig);
        if (!visualInfo) {
            std::cerr << "Cannot get visual info" << std::endl;
            XFree(fbConfigs);
            return false;
        }

        // Create colormap
        m_colormap = XCreateColormap(m_display, RootWindow(m_display, visualInfo->screen), visualInfo->visual, AllocNone);

        // Create window
        XSetWindowAttributes windowAttribs;
        windowAttribs.colormap = m_colormap;
        windowAttribs.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

        m_window = XCreateWindow(
            m_display, RootWindow(m_display, visualInfo->screen),
            0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0,
            visualInfo->depth, InputOutput, visualInfo->visual,
            CWColormap | CWEventMask, &windowAttribs
        );

        if (!m_window) {
            std::cerr << "Cannot create window" << std::endl;
            XFree(visualInfo);
            XFree(fbConfigs);
            return false;
        }

        // Set window title
        XStoreName(m_display, m_window, "OpenGL MSAA Resolve Test - 4K");

        // Set WM_DELETE_WINDOW protocol
        Atom wmDeleteWindow = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(m_display, m_window, &wmDeleteWindow, 1);

        // Map window
        XMapWindow(m_display, m_window);

        // Create OpenGL context
        m_context = glXCreateNewContext(m_display, fbConfig, GLX_RGBA_TYPE, nullptr, True);
        if (!m_context) {
            std::cerr << "Cannot create OpenGL context" << std::endl;
            XFree(visualInfo);
            XFree(fbConfigs);
            return false;
        }

        // Make context current
        if (!glXMakeCurrent(m_display, m_window, m_context)) {
            std::cerr << "Cannot make OpenGL context current" << std::endl;
            XFree(visualInfo);
            XFree(fbConfigs);
            return false;
        }

        // Load OpenGL functions using GLAD
        if (!gladLoadGLLoader((GLADloadproc)glXGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            XFree(visualInfo);
            XFree(fbConfigs);
            return false;
        }

        XFree(visualInfo);
        XFree(fbConfigs);
        return true;
    }

    bool InitializeOpenGL() {
        // Print OpenGL info
        std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

        // Check for required extensions
        const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
        if (!strstr(extensions, "GL_ARB_framebuffer_object")) {
            std::cerr << "GL_ARB_framebuffer_object extension not supported" << std::endl;
            return false;
        }

        // Enable multisampling
        glEnable(GL_MULTISAMPLE);

        return true;
    }

    bool CreateMSAAFramebuffers() {
        // Create MSAA framebuffer
        glGenFramebuffers(1, &m_msaaFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFramebuffer);

        // Create MSAA color texture
        glGenTextures(1, &m_msaaColorTexture);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLE_COUNT, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "MSAA framebuffer incomplete" << std::endl;
            return false;
        }

        // Create resolve framebuffer  
        glGenFramebuffers(1, &m_resolveFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_resolveFramebuffer);

        // Create resolve color texture
        glGenTextures(1, &m_resolveColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_resolveColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resolveColorTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Resolve framebuffer incomplete" << std::endl;
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }

    std::string ReadFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << filename << std::endl;
            return "";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    unsigned int CompileShader(unsigned int type, const std::string& source) {
        unsigned int shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        int result;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE) {
            int length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)alloca(length * sizeof(char));
            glGetShaderInfoLog(shader, length, &length, message);
            std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader: " << message << std::endl;
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    bool LoadShaders() {
        std::string vertexSource = ReadFile("vertex-shader.glsl");
        std::string fragmentSource = ReadFile("fragment-shader.glsl");

        if (vertexSource.empty() || fragmentSource.empty()) {
            return false;
        }

        unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
        unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

        if (vertexShader == 0 || fragmentShader == 0) {
            return false;
        }

        m_shaderProgram = glCreateProgram();
        glAttachShader(m_shaderProgram, vertexShader);
        glAttachShader(m_shaderProgram, fragmentShader);
        glLinkProgram(m_shaderProgram);

        int result;
        glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &result);
        if (result == GL_FALSE) {
            int length;
            glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)alloca(length * sizeof(char));
            glGetProgramInfoLog(m_shaderProgram, length, &length, message);
            std::cerr << "Failed to link shader program: " << message << std::endl;
            glDeleteProgram(m_shaderProgram);
            m_shaderProgram = 0;
            return false;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return true;
    }

    bool CreateVertexArray() {
        glGenVertexArrays(1, &m_vertexArray);
        glBindVertexArray(m_vertexArray);
        // No vertex buffer needed - we use gl_VertexID in the shader
        glBindVertexArray(0);
        return true;
    }

    bool CreateUniformBuffer() {
        glGenBuffers(1, &m_uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(unsigned int) * 4, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        return true;
    }

    void Render() {
        m_frameCount++;

        // Render to MSAA framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFramebuffer);
        glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        
        // Clear to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use shader program
        glUseProgram(m_shaderProgram);
        
        // Update frame count uniform
        unsigned int frameCountLoc = glGetUniformLocation(m_shaderProgram, "frameCount");
        glUniform1ui(frameCountLoc, m_frameCount);

        // Draw fullscreen triangle
        glBindVertexArray(m_vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Resolve MSAA to regular framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolveFramebuffer);
        glBlitFramebuffer(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 
                         0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 
                         GL_COLOR_BUFFER_BIT, GL_LINEAR);

        // Copy resolved texture to back buffer for presentation
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_resolveFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
                         0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
                         GL_COLOR_BUFFER_BIT, GL_LINEAR);

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << error << std::endl;
        }
    }

    void Cleanup() {
        if (m_shaderProgram) {
            glDeleteProgram(m_shaderProgram);
            m_shaderProgram = 0;
        }
        
        if (m_vertexArray) {
            glDeleteVertexArrays(1, &m_vertexArray);
            m_vertexArray = 0;
        }
        
        if (m_uniformBuffer) {
            glDeleteBuffers(1, &m_uniformBuffer);
            m_uniformBuffer = 0;
        }

        if (m_msaaColorTexture) {
            glDeleteTextures(1, &m_msaaColorTexture);
            m_msaaColorTexture = 0;
        }
        
        if (m_resolveColorTexture) {
            glDeleteTextures(1, &m_resolveColorTexture);
            m_resolveColorTexture = 0;
        }
        
        if (m_msaaFramebuffer) {
            glDeleteFramebuffers(1, &m_msaaFramebuffer);
            m_msaaFramebuffer = 0;
        }
        
        if (m_resolveFramebuffer) {
            glDeleteFramebuffers(1, &m_resolveFramebuffer);
            m_resolveFramebuffer = 0;
        }

        if (m_context) {
            glXMakeCurrent(m_display, None, nullptr);
            glXDestroyContext(m_display, m_context);
            m_context = nullptr;
        }

        if (m_window) {
            XDestroyWindow(m_display, m_window);
            m_window = 0;
        }

        if (m_colormap) {
            XFreeColormap(m_display, m_colormap);
            m_colormap = 0;
        }

        if (m_display) {
            XCloseDisplay(m_display);
            m_display = nullptr;
        }
    }
};

int main() {
    std::cout << "OpenGL MSAA Resolve Test - Starting..." << std::endl;

    OpenGLMSAAResolve app;
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize OpenGL MSAA Resolve application" << std::endl;
        return -1;
    }

    std::cout << "Initialization successful. Starting main loop..." << std::endl;
    std::cout << "Press ESC or close window to exit" << std::endl;
    
    app.Run();

    std::cout << "OpenGL MSAA Resolve Test - Finished" << std::endl;
    return 0;
}