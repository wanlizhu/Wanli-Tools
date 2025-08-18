#include "MSAAResolve_GLVK.h"
#define INCBIN_PREFIX g_
#define INCBIN_STYLE  INCBIN_STYLE_SNAKE
#include "incbin.h"

#if !__has_include("vertex-shader.glsl")
#error "Missing vertex-shader.glsl in include path"
#endif 
#if !__has_include("vertex-shader.glsl.spv")
#error "Missing vertex-shader.glsl.spv (generated) in include path"
#endif 

INCTXT(vertex_shader_glsl, "vertex-shader.glsl");
INCTXT(fragment_shader_glsl, "fragment-shader.glsl");
INCBIN(vertex_shader_glsl_spv, "vertex-shader.glsl.spv");
INCBIN(fragment_shader_glsl_spv, "fragment-shader.glsl.spv");

std::unordered_map<std::string, std::vector<uint8_t>> g_files = {
    { "vertex-shader.glsl", std::vector<uint8_t>(g_vertex_shader_glsl_data, g_vertex_shader_glsl_data + g_vertex_shader_glsl_size) },
    { "fragment-shader.glsl", std::vector<uint8_t>(g_fragment_shader_glsl_data, g_fragment_shader_glsl_data + g_fragment_shader_glsl_size) },
    { "vertex-shader.glsl.spv", std::vector<uint8_t>(g_vertex_shader_glsl_spv_data, g_vertex_shader_glsl_spv_data + g_vertex_shader_glsl_spv_size) },
    { "fragment-shader.glsl.spv", std::vector<uint8_t>(g_fragment_shader_glsl_spv_data, g_fragment_shader_glsl_spv_data + g_fragment_shader_glsl_spv_size) }
};

int main(int argc, char** argv) {
    try {
        MSAAResolve_API* app = nullptr;
        /*if (argc > 1 && strcmp(argv[1], "-vk") == 0) {
            app = new MSAAResolve_VK();
        } else {
            app = new MSAAResolve_GL();
        }*/app = new MSAAResolve_VK();

        if (app->Initialize()) {
            app->Run();
            app->Cleanup();
        }

        delete app;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}