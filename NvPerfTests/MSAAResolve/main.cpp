#include "MSAAResolve_GLVK.h"
#define INCBIN_PREFIX g_
#define INCBIN_STYLE  INCBIN_STYLE_SNAKE
#include "incbin.h"

INCTXT(vertex_shader_glsl, "vertex-shader.glsl");
INCTXT(fragment_shader_glsl, "fragment-shader.glsl");
//INCBIN(vertex_shader_glsl_spv, "vertex-shader.glsl.spv");
//INCBIN(fragment_shader_glsl_spv, "fragment-shader.glsl.spv");

std::unordered_map<std::string, std::vector<uint8_t>> g_files = {
    { "vertex-shader.glsl", std::vector<uint8_t>(g_vertex_shader_glsl_data, g_vertex_shader_glsl_data + g_vertex_shader_glsl_size) },
    { "fragment-shader.glsl", std::vector<uint8_t>(g_fragment_shader_glsl_data, g_fragment_shader_glsl_data + g_fragment_shader_glsl_size) },
    //{ "vertex-shader.glsl.spv", std::vector<uint8_t>(g_vertex_shader_glsl_spv_data, g_vertex_shader_glsl_spv_data + g_vertex_shader_glsl_spv_size) },
    //{ "fragment-shader.glsl.spv", std::vector<uint8_t>(g_fragment_shader_glsl_spv_data, g_fragment_shader_glsl_spv_data + g_fragment_shader_glsl_spv_size) }
};

int main(int argc, char** argv) {
    try {
        MSAAResolve_GL app;
        app.Run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}