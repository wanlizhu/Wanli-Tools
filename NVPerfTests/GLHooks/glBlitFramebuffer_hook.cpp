#include "HookUtils.h"

extern "C" void glBlitFramebuffer(
    GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
    GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
    GLbitfield mask, GLenum filter
) {
    static auto real_glBlitFramebuffer = (decltype(&glBlitFramebuffer))dlsym(RTLD_NEXT, "glBlitFramebuffer");
    RunWithGPUTimer("glBlitFramebuffer", [=](){
        real_glBlitFramebuffer(
            srcX0, srcY0, srcX1, srcY1,
            dstX0, dstY0, dstX1, dstY1,
            mask, filter
        );
    });
}