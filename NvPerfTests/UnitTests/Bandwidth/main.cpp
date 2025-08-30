#include "TestCase_BlitFramebuffer.h"
#include "TestCase_Cubemap.h"
#include "TestCase_StorageBuffer.h"
#include "TestCase_Texture2D.h"
#include "TestCase_Texture3D.h"
#include "TestCase_UniformBuffer.h"
#include <iostream>
#include <vector>

#define INCBIN_PREFIX g_
#define INCBIN_STYLE  INCBIN_STYLE_SNAKE
#include "incbin.h"
INCTXT(common_vert_glsl, "Common.vert.glsl");
INCTXT(texture2d_frag_glsl, "TestCase_Texture2D.frag.glsl");
INCTXT(texture3d_frag_glsl, "TestCase_Texture3D.frag.glsl");
INCTXT(cubemap_frag_glsl, "TestCase_Cubemap.frag.glsl");
INCTXT(uniform_buffer_frag_glsl, "TestCase_UniformBuffer.frag.glsl");
INCTXT(storage_buffer_read_frag_glsl, "TestCase_StorageBuffer_read.frag.glsl");
INCTXT(storage_buffer_write_frag_glsl, "TestCase_StorageBuffer_write.frag.glsl");

std::unordered_map<std::string, std::vector<uint8_t>> g_embeddedFiles = {
    { "Common.vert.glsl", std::vector<uint8_t>(g_common_vert_glsl_data, g_common_vert_glsl_data + g_common_vert_glsl_size) },
    { "TestCase_Texture2D.frag.glsl", std::vector<uint8_t>(g_texture2d_frag_glsl_data, g_texture2d_frag_glsl_data + g_texture2d_frag_glsl_size) },
    { "TestCase_Texture3D.frag.glsl", std::vector<uint8_t>(g_texture3d_frag_glsl_data, g_texture3d_frag_glsl_data + g_texture3d_frag_glsl_size) },
    { "TestCase_Cubemap.frag.glsl", std::vector<uint8_t>(g_cubemap_frag_glsl_data, g_cubemap_frag_glsl_data + g_cubemap_frag_glsl_size) },
    { "TestCase_UniformBuffer.frag.glsl", std::vector<uint8_t>(g_uniform_buffer_frag_glsl_data, g_uniform_buffer_frag_glsl_data + g_uniform_buffer_frag_glsl_size) },
    { "TestCase_StorageBuffer_read.frag.glsl", std::vector<uint8_t>(g_storage_buffer_read_frag_glsl_data, g_storage_buffer_read_frag_glsl_data + g_storage_buffer_read_frag_glsl_size) },
    { "TestCase_StorageBuffer_write.frag.glsl", std::vector<uint8_t>(g_storage_buffer_write_frag_glsl_data, g_storage_buffer_write_frag_glsl_data + g_storage_buffer_write_frag_glsl_size) },
};

int main(int argc, char** argv) {
    std::vector<TestResult> results;

    UnitTest_BW_BlitFramebuffer_GL().Run(results);
    UnitTest_BW_Texture2D_GL().Run(results);
    UnitTest_BW_Texture3D_GL().Run(results);
    UnitTest_BW_Cubemap_GL().Run(results);
    UnitTest_BW_UniformBuffer_GL().Run(results);
    UnitTest_BW_StorageBuffer_GL().Run(results);
    
    size_t maxTestNameLen = 0;
    for (const auto& result : results) {
        maxTestNameLen = std::max(maxTestNameLen, result.testName.length());
    }
    
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& result : results) {
        std::cout << "[" << std::left << std::setw(maxTestNameLen) << result.testName << "] " 
                  << std::setw(5) << result.operation << " Bandwidth: " 
                  << std::setw(10) << std::left << result.bandwidth_gbps << " GB/s | "
                  << "FPS: " << result.fps << std::endl;
    }
    
    return 0;
}