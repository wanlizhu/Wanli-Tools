// Define and include GL implementation
#define GLAD_GL_IMPLEMENTATION
#include "gl.h"
#undef GLAD_GL_IMPLEMENTATION  

#define GLAD_GLX_IMPLEMENTATION      
#define GLAD_EGL_IMPLEMENTATION  
#include "glx.h" // X11
#include "egl.h" // Wayland

