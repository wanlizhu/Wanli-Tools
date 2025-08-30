#include "MSAAResolve_GLVK.h"
/*
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

bool MSAAResolve_VK::Initialize() {
    try {
        InitializeGLFW();
        CreateInstance();
        CreateLogicalDevice();
        CreateSwapchain();
        CreateRenderPass();
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
        CreateMSAAFramebuffers();
        CreateDescriptorSet();
        CreateCommandBuffer();
        CreateSyncObjects();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    std::cout << "Initialization Finished" << std::endl;
    return true;
}

void MSAAResolve_VK::Run() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        }

        Render();
    }
}

void MSAAResolve_VK::Cleanup() {
    vkDeviceWaitIdle(m_device);

    if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);
    }
    if (m_renderFinishedSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr);
    }
    if (m_inFlightFence != VK_NULL_HANDLE) {
        vkDestroyFence(m_device, m_inFlightFence, nullptr);
    }
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    }
    if (m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    }
    if (m_uniformBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device, m_uniformBuffer, nullptr);
    }
    if (m_uniformBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, m_uniformBufferMemory, nullptr);
    }
    if (m_graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
    }
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    }
    if (m_descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    }
    for (VkFramebuffer framebuffer : m_framebuffers) {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }
    }
    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    }
    if (m_msaaColorImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device, m_msaaColorImageView, nullptr);
    }
    if (m_msaaColorImage != VK_NULL_HANDLE) {
        vkDestroyImage(m_device, m_msaaColorImage, nullptr);
    }
    if (m_msaaColorImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, m_msaaColorImageMemory, nullptr);
    }
    if (m_resolveImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device, m_resolveImageView, nullptr);
    }
    if (m_resolveImage != VK_NULL_HANDLE) {
        vkDestroyImage(m_device, m_resolveImage, nullptr);
    }
    if (m_resolveImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, m_resolveImageMemory, nullptr);
    }
    for (VkImageView imageView : m_swapchainImageViews) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, imageView, nullptr);
        }
    }
    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    }
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
    }
    if (m_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    }
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
    }

    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
    std::cout << "Cleanup Finished" << std::endl;
}

void GLFW_ErrorCallback_VK(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void MSAAResolve_VK::InitializeGLFW() {
    glfwSetErrorCallback(GLFW_ErrorCallback_VK);
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    if (!glfwVulkanSupported()) {
        throw std::runtime_error("Vulkan is not supported");
    }

    // Set up window hints
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLE_COUNT);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    m_window = glfwCreateWindow(TEXTURE_WIDTH, TEXTURE_HEIGHT, "Vulkan MSAA Resolve Test", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    std::cout << "GLFW window created with " << MSAA_SAMPLE_COUNT << "x MSAA" << std::endl;
}

bool FindValidationLayers() {
    static bool cachedResult = false;
    static bool inited = false;

    if (inited) {
        return cachedResult;
    }

    inited = true;
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                std::cout << "Found " << layerName << std::endl;
            }
        }

        if (!layerFound) {
            std::cout << "Validation layer disabled" << std::endl;
            cachedResult = false;
            return false;
        }
    }

    std::cout << "Validation layer enabled" << std::endl;
    cachedResult = true;
    return true;
}

std::vector<const char*> GetRequiredInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (FindValidationLayers()) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan_ErrorCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT messageType, 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
    void* pUserData
) {
    std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void MSAAResolve_VK::CreateInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequiredInstanceExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (FindValidationLayers()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = Vulkan_ErrorCallback;
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    
    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create instance");
    }

    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }

    std::cout << "Created vulkan instance" << std::endl;
}

VkPhysicalDevice FindBestPhysicalDevice(VkInstance instance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    uint32_t deviceIndex = UINT32_MAX;
    for (int i = 0; i < (int)deviceCount; i++) {
        VkPhysicalDeviceProperties props = {};
        vkGetPhysicalDeviceProperties(devices[i], &props);

        std::cout << "Physical device " << i << ": " << props.deviceName <<
            " (API Version: " << VK_VERSION_MAJOR(props.apiVersion) 
            << "." << VK_VERSION_MINOR(props.apiVersion) 
            << "." << VK_VERSION_PATCH(props.apiVersion) << ")" << std::endl;

        if (deviceIndex == UINT32_MAX) {
            if (props.vendorID == 0x10DE) { // NVIDIA GPU
                deviceIndex = i;
            }
        }
    }

    if (deviceIndex == UINT32_MAX) {
        throw std::runtime_error("Failed to find a suitable GPU");
    } else {
        std::cout << "Select physical device: " << deviceIndex << std::endl;
    }

    return devices[deviceIndex];
}

uint32_t FindBestQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    uint32_t bestIndex = UINT32_MAX;
    for (int i = 0; i < (int)queueFamilyCount; i++) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        std::cout << "Queue family index " << i << ": " 
            << ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ? "G" : ".") 
            << ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) ? "T" : ".")
            << ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) ? "C" : ".")
            << (presentSupport ? "P " : ".") << std::endl;

        if (bestIndex == UINT32_MAX) {
            if (queueFamilies[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)) {
                if (presentSupport) {
                    bestIndex = i;
                }
            }
        }
    }

    if (bestIndex == UINT32_MAX) {
        throw std::runtime_error("Failed to find an All-in-One queue family index");
    } else {
        std::cout << "Select queue family index: " << bestIndex << std::endl;
    }

    return bestIndex;
}

void MSAAResolve_VK::CreateLogicalDevice() {
    m_physicalDevice = FindBestPhysicalDevice(m_instance);
    m_queueFamilyIndex = FindBestQueueFamilyIndex(m_physicalDevice, m_surface);

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};
    
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pEnabledFeatures = &deviceFeatures;

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(m_device, m_queueFamilyIndex, 0, &m_queue);
    
    std::cout << "Created logical device" << std::endl;
}

VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView = VK_NULL_HANDLE;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image view");
    }

    return imageView;
}

void MSAAResolve_VK::CreateSwapchain() {
    VkSurfaceCapabilitiesKHR capabilities {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    
    // Respect the surface capabilities for image count
    uint32_t imageCount = capabilities.minImageCount;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    createInfo.minImageCount = imageCount;
    
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.preTransform = capabilities.currentTransform;
    
    std::cout << "Surface capabilities: minImageCount=" << capabilities.minImageCount 
              << ", maxImageCount=" << capabilities.maxImageCount 
              << ", using imageCount=" << imageCount << std::endl;
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        createInfo.imageExtent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        createInfo.imageExtent = actualExtent;
    }

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());
    createInfo.imageFormat = VK_FORMAT_UNDEFINED;
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM ||
            availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
            createInfo.imageFormat = availableFormat.format;
            createInfo.imageColorSpace = availableFormat.colorSpace;
            break;
        }
    }
    if (createInfo.imageFormat == VK_FORMAT_UNDEFINED) {
        throw std::runtime_error("Surface doesn't support VK_FORMAT_R8G8B8A8_UNORM nor VK_FORMAT_B8G8R8A8_UNORM");
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data());
    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            createInfo.presentMode = availablePresentMode;
        }
    }

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain");
    }

    m_swapchainImageFormat = createInfo.imageFormat;
    m_swapchainImageExtent = createInfo.imageExtent;

    imageCount = 0;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

    m_swapchainImageViews.resize(imageCount);
    for (int i = 0; i < (int)m_swapchainImages.size(); i++) {
        m_swapchainImageViews[i] = CreateImageView(m_device, m_swapchainImages[i], m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    std::cout << "Created swapchain with " << m_swapchainImages.size() << " backbuffers of " << m_swapchainImageExtent.width << "x" << m_swapchainImageExtent.height << std::endl;
}

void MSAAResolve_VK::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainImageFormat;
    colorAttachment.samples = static_cast<VkSampleCountFlagBits>(MSAA_SAMPLE_COUNT);
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription resolveAttachment{};
    resolveAttachment.format = m_swapchainImageFormat;
    resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VkAttachmentReference resolveAttachmentRef{};
    resolveAttachmentRef.attachment = 1;
    resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pResolveAttachments = &resolveAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[2];
    attachments[0] = colorAttachment;
    attachments[1] = resolveAttachment;
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }

    std::cout << "Created render pass" << std::endl;
}

void MSAAResolve_VK::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }

    std::cout << "Created descriptor set layout" << std::endl;
}

VkShaderModule CreateShaderModule(VkDevice device, VkShaderStageFlagBits stage, const std::string& filename) {
    std::vector<unsigned char> bytes;
    if (std::filesystem::exists(filename)) {
        std::cout << "Load shader source from file: " << filename << std::endl;
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << filename << std::endl;
            throw std::runtime_error("Cannot open file");
        }

        bytes.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        if (!file.read((char*)bytes.data(), bytes.size())) {
            throw std::runtime_error("Failed to read .spv file");
        }
    }
    else {
        std::cout << "Load shader source from embedded string: " << filename << std::endl;
        bytes = g_files.at(filename);
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytes.size();
    createInfo.pCode = (const uint32_t*)bytes.data();

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }

    return shaderModule;
}

void MSAAResolve_VK::CreateGraphicsPipeline() {
    VkShaderModule vertShaderModule = CreateShaderModule(m_device, VK_SHADER_STAGE_VERTEX_BIT, "vertex-shader.glsl.spv");
    VkShaderModule fragShaderModule = CreateShaderModule(m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "fragment-shader.glsl.spv");
    
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    
    if (vertShaderModule != VK_NULL_HANDLE) {
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        shaderStages.push_back(vertShaderStageInfo);
    }
    
    if (fragShaderModule != VK_NULL_HANDLE) {
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";
        shaderStages.push_back(fragShaderStageInfo);
    }
    
    // Vertex input (none needed for fullscreen triangle)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapchainImageExtent.width;
    viewport.height = (float)m_swapchainImageExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchainImageExtent;
    
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = static_cast<VkSampleCountFlagBits>(MSAA_SAMPLE_COUNT);
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
    
    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }
    
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    
    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
    
    if (vertShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
    }

    if (fragShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    }

    std::cout << "Created graphics pipeline" << std::endl;
}

uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
    return 0;
}

void MSAAResolve_VK::CreateMSAAFramebuffers() {
    VkImageCreateInfo msaaImageInfo{};
    msaaImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    msaaImageInfo.imageType = VK_IMAGE_TYPE_2D;
    msaaImageInfo.extent.width = m_swapchainImageExtent.width;
    msaaImageInfo.extent.height = m_swapchainImageExtent.height;
    msaaImageInfo.extent.depth = 1;
    msaaImageInfo.mipLevels = 1;
    msaaImageInfo.arrayLayers = 1;
    msaaImageInfo.format = m_swapchainImageFormat;
    msaaImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    msaaImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    msaaImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    msaaImageInfo.samples = static_cast<VkSampleCountFlagBits>(MSAA_SAMPLE_COUNT);
    msaaImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_device, &msaaImageInfo, nullptr, &m_msaaColorImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create MSAA color image");
    }

    VkMemoryRequirements msaaMemRequirements;
    vkGetImageMemoryRequirements(m_device, m_msaaColorImage, &msaaMemRequirements);

    VkMemoryAllocateInfo msaaAllocInfo{};
    msaaAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    msaaAllocInfo.allocationSize = msaaMemRequirements.size;
    msaaAllocInfo.memoryTypeIndex = FindMemoryType(m_physicalDevice, msaaMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_device, &msaaAllocInfo, nullptr, &m_msaaColorImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate MSAA image memory");
    }

    vkBindImageMemory(m_device, m_msaaColorImage, m_msaaColorImageMemory, 0);

    m_msaaColorImageView = CreateImageView(m_device, m_msaaColorImage, m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageCreateInfo resolveImageInfo = msaaImageInfo;
    resolveImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    resolveImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (vkCreateImage(m_device, &resolveImageInfo, nullptr, &m_resolveImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create resolve image");
    }

    VkMemoryRequirements resolveMemRequirements;
    vkGetImageMemoryRequirements(m_device, m_resolveImage, &resolveMemRequirements);

    VkMemoryAllocateInfo resolveAllocInfo{};
    resolveAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    resolveAllocInfo.allocationSize = resolveMemRequirements.size;
    resolveAllocInfo.memoryTypeIndex = FindMemoryType(m_physicalDevice, resolveMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_device, &resolveAllocInfo, nullptr, &m_resolveImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate resolve image memory");
    }

    vkBindImageMemory(m_device, m_resolveImage, m_resolveImageMemory, 0);

    m_resolveImageView = CreateImageView(m_device, m_resolveImage, m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    m_framebuffers.resize(m_swapchainImages.size());
    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        VkImageView attachments[2];
        attachments[0] = m_msaaColorImageView;
        attachments[1] = m_resolveImageView;

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchainImageExtent.width;
        framebufferInfo.height = m_swapchainImageExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }

    std::cout << "Created MSAA framebuffers" << std::endl;
}

void MSAAResolve_VK::CreateDescriptorSet() {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint32_t) * 4; // frameIndex + padding
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_uniformBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create uniform buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, m_uniformBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(m_physicalDevice, memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_uniformBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate uniform buffer memory");
    }

    vkBindBufferMemory(m_device, m_uniformBuffer, m_uniformBufferMemory, 0);
    vkMapMemory(m_device, m_uniformBufferMemory, 0, bufferInfo.size, 0, &m_uniformBufferMapped);

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }

    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = m_descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vkAllocateDescriptorSets(m_device, &allocateInfo, &m_descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set");
    }

    VkDescriptorBufferInfo bufferDescInfo{};
    bufferDescInfo.buffer = m_uniformBuffer;
    bufferDescInfo.offset = 0;
    bufferDescInfo.range = sizeof(uint32_t) * 4;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferDescInfo;

    vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);

    std::cout << "Created descriptor set" << std::endl;
}

void MSAAResolve_VK::CreateCommandBuffer() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_queueFamilyIndex;

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer");
    }

    std::cout << "Created command buffer" << std::endl;
}

void MSAAResolve_VK::CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create synchronization objects");
    }

    std::cout << "Created sync objects" << std::endl;
}

void MSAAResolve_VK::Render() {
    // Wait for the previous frame to finish
    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    
    // Acquire next swapchain image
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    // Now it's safe to reset the fence since we've waited for it
    vkResetFences(m_device, 1, &m_inFlightFence);

    // Update uniform buffer with frame count
    m_frameIndex++;
    uint32_t uniformData[4] = { m_frameIndex, 0, 0, 0 }; // frameIndex + padding
    memcpy(m_uniformBufferMapped, uniformData, sizeof(uniformData));

    // Reset command buffer after waiting for fence
    vkResetCommandBuffer(m_commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapchainImageExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; // Clear to black 
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
    vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
    vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(m_commandBuffer);

    // Copy resolved image to swapchain image for presentation
    // Transition swapchain image to transfer destination
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_swapchainImages[imageIndex];
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Copy resolved image to swapchain image (like DX12 CopyTextureRegion)
    VkImageCopy copyRegion{};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent = {m_swapchainImageExtent.width, m_swapchainImageExtent.height, 1};

    vkCmdCopyImage(m_commandBuffer, m_resolveImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_swapchainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    // Transition swapchain image to present layout
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_queue, 1, &submitInfo, m_inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_queue, &presentInfo);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to present swapchain image");
    }
}
*/