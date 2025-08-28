#pragma once
#include "glad/gl.h"
#include "glad/glx.h" // X11
#include "glad/egl.h" // Wayland
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <map>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <unordered_map>

// Global embedded files
extern std::unordered_map<std::string, std::vector<uint8_t>> g_files;

class MSAAResolve_API {
public:
    static const unsigned int TEXTURE_WIDTH = 3840;
    static const unsigned int TEXTURE_HEIGHT = 2160;
    static const unsigned int MSAA_SAMPLE_COUNT = 8;

    virtual ~MSAAResolve_API() {}
    virtual bool Initialize() = 0;
    virtual void Run() = 0;
    virtual void Cleanup() = 0;
};

class MSAAResolve_GL : public MSAAResolve_API {
public:
    virtual bool Initialize() override;
    virtual void Run() override;
    virtual void Cleanup() override;

private:
    void InitializeGLFW();
    void CreateMSAAFramebuffers();
    void CreateShaderProgram();
    void CreateVertexArray();
    void CreateUniformBuffer();
    void Render();

private:
    GLFWwindow* m_window = nullptr;
    GLuint m_msaaFramebuffer = 0;
    GLuint m_msaaColorTexture = 0;
    GLuint m_resolveFramebuffer = 0;
    GLuint m_resolveColorTexture = 0;
    GLuint m_shaderProgram = 0;
    GLuint m_vertexArray = 0;
    GLuint m_uniformBuffer = 0;
    int m_frameCount = 0;
    int m_actualMSAASamples = 0;
};

class MSAAResolve_VK : public MSAAResolve_API {
public:
    virtual bool Initialize() override;
    virtual void Run() override;
    virtual void Cleanup() override;

private:
    void InitializeGLFW();
    void CreateInstance();
    void CreateLogicalDevice();
    void CreateSwapchain();
    void CreateRenderPass();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateMSAAFramebuffers();
    void CreateDescriptorSet();
    void CreateCommandBuffer();
    void CreateSyncObjects();
    void Render();

private:
    GLFWwindow* m_window = nullptr;
    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    uint32_t m_queueFamilyIndex = 0;
    VkQueue m_queue = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapchainImageExtent = {};
    std::vector<VkImage> m_swapchainImages = {};
    std::vector<VkImageView> m_swapchainImageViews = {};
    
    // MSAA resources
    VkImage m_msaaColorImage = VK_NULL_HANDLE;
    VkDeviceMemory m_msaaColorImageMemory = VK_NULL_HANDLE;
    VkImageView m_msaaColorImageView = VK_NULL_HANDLE;
    VkImage m_resolveImage = VK_NULL_HANDLE;
    VkDeviceMemory m_resolveImageMemory = VK_NULL_HANDLE;
    VkImageView m_resolveImageView = VK_NULL_HANDLE;
    
    // Render pass and framebuffers
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers = {};
    
    // Pipeline
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    
    // Descriptor set and uniform buffer
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    VkBuffer m_uniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_uniformBufferMemory = VK_NULL_HANDLE;
    void* m_uniformBufferMapped = nullptr;
    
    // Command buffer
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    
    // Synchronization
    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence m_inFlightFence = VK_NULL_HANDLE;
    
    // Frame count for animation
    uint32_t m_frameCount = 0;
};