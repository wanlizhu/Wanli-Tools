#include "MSAAResolve_GLVK.h"

INCTXT(vertex_shader_glsl, "vertex-shader.glsl");
INCTXT(fragment_shader_glsl, "fragment-shader.glsl");
INCTXT(vertex_shader_glsl_spv, "vertex-shader.glsl.spv");
INCTXT(fragment_shader_glsl_spv, "fragment-shader.glsl.spv");

std::unordered_map<std::string, std::string> g_files = {
    { "vertex-shader.glsl", std::string(g_vertex_shader_glsl_data, g_vertex_shader_glsl_size) },
    { "fragment-shader.glsl", std::string(g_fragment_shader_glsl_data, g_fragment_shader_glsl_size) },
    { "vertex-shader.glsl.spv", std::string(g_vertex_shader_glsl_spv_data, g_vertex_shader_glsl_spv_size) },
    { "fragment-shader.glsl.spv", std::string(g_fragment_shader_glsl_spv_data, g_fragment_shader_glsl_spv_size) }
};

int main(int argc, char** argv) {
    try {
        MSAAResolve_API* app = nullptr;
        if (argc > 1 || strcmp(argv[1], "-vk") == 0) {
            app = new MSAAResolve_VK();
        } else {
            app = new MSAAResolve_GL();
        }

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