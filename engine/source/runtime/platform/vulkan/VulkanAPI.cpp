#include "runtime/platform/vulkan/VulkanAPI.h"
#include "runtime/code/base/macro.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define DYNASTY_XSTR(s) DYNASTY_STR(s)
#define DYNASTY_STR(s) #s 

#include <algorithm>
#include <cmath>

#if defined(__GNUC__)
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if defined(__linux__)
#include <stdlib.h>
#elif defined(__MACH__)
// https://developer.apple.com/library/archive/documentation/Porting/Conceptual/PortingUnix/compiling/compiling.html
#include <stdlib.h>
#else
#error Unknown Platform
#endif
#elif defined(_MSC_VER)
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN 1
#define NOGDICAPMASKS 1
#define NOVIRTUALKEYCODES 1
#define NOWINMESSAGES 1
#define NOWINSTYLES 1
#define NOSYSMETRICS 1
#define NOMENUS 1
#define NOICONS 1
#define NOKEYSTATES 1
#define NOSYSCOMMANDS 1
#define NORASTEROPS 1
#define NOSHOWWINDOW 1
#define NOATOM 1
#define NOCLIPBOARD 1
#define NOCOLOR 1
#define NOCTLMGR 1
#define NODRAWTEXT 1
#define NOGDI 1
#define NOKERNEL 1
#define NOUSER 1
#define NONLS 1
#define NOMB 1
#define NOMEMMGR 1
#define NOMETAFILE 1
#define NOMINMAX 1
#define NOMSG 1
#define NOOPENFILE 1
#define NOSCROLL 1
#define NOSERVICE 1
#define NOSOUND 1
#define NOTEXTMETRIC 1
#define NOWH 1
#define NOWINOFFSETS 1
#define NOCOMM 1
#define NOKANJI 1
#define NOHELP 1
#define NOPROFILER 1
#define NODEFERWINDOWPOS 1
#define NOMCX 1
#include <Windows.h>
#else
#error Unknown Compiler
#endif

#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>


namespace DynastyEngine {
    // https://stackoverflow.com/questions/68127785/how-to-fix-vk-khr-portability-subset-error-on-mac-m1-while-following-vulkan-tuto
    // const std::vector<const char *> kRequiredInstanceExtensions = 
    // {
    // #ifdef PLATFORM_OSX
    //     VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
    // #endif
    //     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    // };

    // const std::vector<const char *> kRequiredDeviceExtensions = 
    // {
    // #ifdef PLATFORM_OSX
    //     VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
    // #endif
    // };

    // debug callback
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                        VkDebugUtilsMessageTypeFlagsEXT,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void*)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    void VulkanAPI::initialize(VulkanInitInfo initInfo)
    {   
        mWindow = initInfo.windowSystem->getWindow();
        std::array<int, 2> windowSize = initInfo.windowSystem->getWindowSize();

        mViewport = {0.0f, 0.0f, (float)windowSize[0], (float)windowSize[1], 0.0f, 1.0f};
        mScissor  = {{0, 0}, {(uint32_t)windowSize[0], (uint32_t)windowSize[1]}};

#ifndef NDEBUG
    pEnableValidationLayers = true;
    pEnableDebugUtilsLabel = true;
#else
    pEnableValidationLayers  = false;
    pEnableDebugUtilsLabel  = false;
#endif

#if defined(__GNUC__)
    // https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
    #if defined(__linux__)
        char const* vk_layer_path = DYNASTY_XSTR(DYNASTY_VK_LAYER_PATH);
        setenv("VK_LAYER_PATH", vk_layer_path, 1);
    #elif defined(__MACH__)
        // https://developer.apple.com/library/archive/documentation/Porting/Conceptual/PortingUnix/compiling/compiling.html
        char const* vk_layer_path    = DYNASTY_XSTR(DYNASTY_VK_LAYER_PATH);
        char const* vk_icd_filenames = DYNASTY_XSTR(DYNASTY_VK_ICD_FILENAMES);
        setenv("VK_LAYER_PATH", vk_layer_path, 1);
        setenv("VK_ICD_FILENAMES", vk_icd_filenames, 1);
    #else
        #error Unknown Platform
    #endif
#elif defined(_MSC_VER)
    // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
    char const* vk_layer_path = DYNASTY_XSTR(DYNASTY_VK_LAYER_PATH);
    SetEnvironmentVariableA("VK_LAYER_PATH", vk_layer_path);
#else
    #error Unknown Compiler
#endif

        createInstance();

        LOG_INFO("initializeDebugMessenger!");
        initializeDebugMessenger();

        LOG_INFO("createWindowSurface!");
        createWindowSurface();

        LOG_INFO("initializePhysicalDevice!");
        initializePhysicalDevice();

        LOG_INFO("createLogicalDevice!");
        createLogicalDevice();

        LOG_INFO("createCommandPool!");
        createCommandPool();

        LOG_INFO("createCommandBuffers!");
        createCommandBuffers();

        LOG_INFO("createDescriptorPool!");
        createDescriptorPool();

        LOG_INFO("createSyncPrimitives!");
        createSyncPrimitives();

        LOG_INFO("createSwapchain!");
        createSwapchain();

        LOG_INFO("createSwapchainImageViews!");
        createSwapchainImageViews();

        LOG_INFO("createFramebufferImageAndView!");
        createFramebufferImageAndView();

        LOG_INFO("createVmaAllocator!");
        createVmaAllocator();

        LOG_INFO("创建完成！");
    }

    void VulkanAPI::createInstance() 
    {
        if (pEnableValidationLayers && !checkValidationLayerSupport()) 
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Dynasty";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Dynasty Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (pEnableValidationLayers) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
            createInfo.ppEnabledLayerNames = mValidationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } 
        else 
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) 
        {
            LOG_ERROR("failed to create instance!");
        }
    }
    
    bool VulkanAPI::initializeDebugMessenger() 
    {
        if (!pEnableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (createDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
            return false;
        } 
        
        return true;
    }

    void VulkanAPI::createWindowSurface() 
    {
        if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
        {
            std::runtime_error("glfwCreateWindowSurface failed!");
        }
    }
        
    void VulkanAPI::initializePhysicalDevice() 
    {   
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

        if (deviceCount == 0) 
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(mInstance, &deviceCount, physicalDevices.data());

        std::vector<std::pair<int, VkPhysicalDevice>> rankedPhysicalDevices;
        for (const auto& device : physicalDevices)
        {
            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
            int score = 0;

            if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                score += 1000;
            }
            else if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                score += 100;
            }

            rankedPhysicalDevices.push_back({score, device});
        }

        std::sort(rankedPhysicalDevices.begin(),
                  rankedPhysicalDevices.end(),
                  [](const std::pair<int, VkPhysicalDevice>& p1, const std::pair<int, VkPhysicalDevice>& p2) {
                    return p1 > p2;
                  });

        for (const auto& device : rankedPhysicalDevices)
        {
            LOG_INFO("isDeviceSuitable");
            if (isDeviceSuitable(device.second))
            {
                mPhysicalDevice = device.second;
                break;
            }
        }

        if (mPhysicalDevice == VK_NULL_HANDLE)
        {
            LOG_ERROR("failed to find suitable physical device");
        }
    }

    void VulkanAPI::createLogicalDevice() 
    {
        mQueueIndices = findQueueFamilies(mPhysicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {mQueueIndices.graphicsFamily.value(), mQueueIndices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) 
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
        // support inefficient readback storage buffer
        physicalDeviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
        // support independent blending
        physicalDeviceFeatures.independentBlend = VK_TRUE;
        // // support geometry shader
        // if (mEnablePointLightShadow)
        // {
        //     physicalDeviceFeatures.geometryShader = VK_TRUE;
        // }

#if defined(__MACH__)
        mDeviceExtensions.push_back("VK_KHR_portability_subset");
#endif
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount     = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos        = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures         = &physicalDeviceFeatures;
        deviceCreateInfo.enabledExtensionCount    = static_cast<uint32_t>(mDeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames  = mDeviceExtensions.data();

        if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(mDevice, mQueueIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
        vkGetDeviceQueue(mDevice, mQueueIndices.presentFamily.value(), 0, &mPresentQueue);

        mDepthImageFormat = findDepthFormat();
    }
        
    void VulkanAPI::createSwapchain() 
    {
        SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(mPhysicalDevice);

        // 如果满足了swapChainAdequate的条件，那么支持肯定是足够的，但是仍然可能有许多不同的模式，有不同的优化。我们现在要写几个函数来找到最佳交换链的正确设置。有三种类型的设置需要确定。
        VkSurfaceFormatKHR chosenSurfaceFormat = chooseSwapchainSurfaceFormatFromDetails(swapChainSupportDetails.formats);
        VkPresentModeKHR chosenPresentMode = chooseSwapchainPresentModeFromDetails(swapChainSupportDetails.presentModes);
        VkExtent2D chosenExtent = chooseSwapchainExtentFromDetails(swapChainSupportDetails.capabilities); // 看看这个有影响吗

        // 除了这些属性之外，我们还必须决定我们希望在交换链中拥有多少图像
        // 我们建议至少要比最小值多请求一个图像。
        uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
        //不要超过最大的图片数量，其中0是一个特殊的值，意味着没有最大的数量。
        if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount) 
        {
            imageCount = swapChainSupportDetails.capabilities.maxImageCount;
        }

        // 创建交换链对象需要填写一个大的结构
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = mSurface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = chosenSurfaceFormat.format;
        createInfo.imageColorSpace = chosenSurfaceFormat.colorSpace;
        createInfo.imageExtent = chosenExtent;
        // 每个图像所包含的层数。除非你正在开发一个立体的3D应用程序，否则这总是1
        createInfo.imageArrayLayers = 1;
        // 我们将在交换链中使用图像的哪种操作
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = {mQueueIndices.graphicsFamily.value(), mQueueIndices.presentFamily.value()};
        // vk_sharing_mode_exclusive。一个图像一次由一个队列家族拥有，在另一个队列家族中使用它之前，必须明确转移所有权。这个选项提供了最好的性能。
        // vk_sharing_mode_concurrent。图像可以在多个队列家族中使用，无需明确的所有权转移。
        if (mQueueIndices.graphicsFamily != mQueueIndices.presentFamily) 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } 
        else 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        // 我们可以指定在交换链中，如果支持某种变换（capabilities中的supportedTransforms
        // 要指定你不想要任何变换，只需指定当前的变换。
        createInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform;
        // 是否应该使用alpha通道与窗口系统中的其他窗口进行混合
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = chosenPresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain) != VK_SUCCESS) 
        {
            LOG_ERROR("failed to create swap chain!");
        }

        // 在createSwapChain'函数的末尾添加了检索句柄的代码
        // 我们只指定了交换链中图像的最低数量，所以实现允许创建一个有更多图像的交换链。
        // 先用vkGetSwapchainImagesKHR查询最终的图像数量，然后调整容器的大小，最后再调用它来检索手柄。
        vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
        mSwapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

        // 将我们为交换链图像选择的格式和范围存储在成员变量中。
        mSwapchainImageFormat = chosenSurfaceFormat.format;
        mSwapchainExtent.height = chosenExtent.height;
        mSwapchainExtent.width = chosenExtent.width;

        mScissor = {{0, 0}, {mSwapchainExtent.width, mSwapchainExtent.height}};
    }
        
    void VulkanAPI::recreateSwapchain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(mWindow, &width, &height);
        while (width == 0 || height == 0) 
        {
            glfwGetFramebufferSize(mWindow, &width, &height);
            glfwWaitEvents();
        }

        VkResult resWaitForFences = vkWaitForFences(mDevice, kMaxFramesInFlight, mIsFrameInFlightFences, VK_TRUE, UINT64_MAX);
        if (VK_SUCCESS != resWaitForFences)
        {
            LOG_ERROR("_vkWaitForFences failed");
            return;
        }
        vkDeviceWaitIdle(mDevice);

        destroyImageView(mDepthImageView);
        vkDestroyImage(mDevice, mDepthImage, NULL);
        vkFreeMemory(mDevice, mDepthImageMemory, NULL);

        for (auto imageview : mSwapchainImageviews)
        {
            vkDestroyImageView(mDevice, imageview, NULL);
        }
        vkDestroySwapchainKHR(mDevice, mSwapchain, NULL);

        createSwapchain();
        createSwapchainImageViews();
        createFramebufferImageAndView();
    }

    void VulkanAPI::createSwapchainImageViews()
    {
        // 调整列表的大小，以适应我们将要创建的所有图像视图。
        mSwapchainImageviews.resize(mSwapchainImages.size());

        // 设置循环，在所有交换链图像上进行迭代。
        for (size_t i = 0; i < mSwapchainImages.size(); i++) 
        {
            VkImageViewCreateInfo createInfo{};
            // 创建图像视图的参数在VkImageViewCreateInfo结构中指定。前面的几个参数是直接的。
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = mSwapchainImages[i];
            // viewType 和 format字段指定了图像数据的解释方式。
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = mSwapchainImageFormat;
            // components字段允许你对颜色通道进行旋转,我们将坚持使用默认的映射。
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            // subresourceRange字段描述了图像的目的是什么，应该访问图像的哪一部分。
            // 我们的图像将被用作颜色目标，没有任何mipmapping级别或多个层次。
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapchainImageviews[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }
        
    void VulkanAPI::createFramebufferImageAndView() {
        createImage(mSwapchainExtent.width,
                    mSwapchainExtent.height,
                    mDepthImageFormat,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    mDepthImage,
                    mDepthImageMemory,
                    0, 
                    1, 
                    1);
        createImageView(mDepthImage, 
                        mDepthImageFormat,
                        VK_IMAGE_ASPECT_DEPTH_BIT, 
                        VK_IMAGE_VIEW_TYPE_2D,
                        1,
                        1,
                        mDepthImageView);
    }

    void VulkanAPI::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferCreateInfo {};
        bufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size        = size;
        bufferCreateInfo.usage       = usage;                     // use as a vertex/staging/index buffer
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing among queue families

        if (vkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            LOG_ERROR("vkCreateBuffer failed!");
            return;
        }

        VkMemoryRequirements bufferMemoryRequirements; // for allocate_info.allocationSize and
                                                         // allocate_info.memoryTypeIndex
        vkGetBufferMemoryRequirements(mDevice, buffer, &bufferMemoryRequirements);

        VkMemoryAllocateInfo bufferMemoryAllocateInfo {};
        bufferMemoryAllocateInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        bufferMemoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;
        bufferMemoryAllocateInfo.memoryTypeIndex = findMemoryType(mPhysicalDevice, bufferMemoryRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(mDevice, &bufferMemoryAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            LOG_ERROR("vkAllocateMemory failed!");
            return;
        }

        // bind buffer with buffer memory
        vkBindBufferMemory(mDevice, buffer, bufferMemory, 0); // offset = 0
    }
        
    bool VulkanAPI::createBufferVMA(VmaAllocator allocator, const 
                                    VkBufferCreateInfo pBufferCreateInfo, 
                                    const VmaAllocationCreateInfo* pAllocationCreateInfo, 
                                    VkBuffer &pBuffer, 
                                    VmaAllocation* pAllocation, 
                                    VmaAllocationInfo* pAllocationInfo)
    {
        VkResult result = vmaCreateBuffer(allocator, &pBufferCreateInfo, pAllocationCreateInfo, &pBuffer, pAllocation, pAllocationInfo);
        if (result == VK_SUCCESS)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
        
    bool VulkanAPI::copyBuffer(VkBuffer     srcBuffer,
                               VkBuffer     dstBuffer,
                               VkDeviceSize srcOffset,
                               VkDeviceSize dstOffset,
                               VkDeviceSize size) 
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        
        VkBufferCopy copyRegion = {srcOffset, dstOffset, size};
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }
    
    void VulkanAPI::createShaderModule(VkShaderModule  &shaderModule, const std::vector<char>& code) 
    {
        // 该函数将接收一个带有字节码的缓冲区作为参数，并从中创建一个VkShaderModule。
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        // 只需要指定一个带有字节码的缓冲区的指针和它的长度
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create shader module!");
        }
    }

    void VulkanAPI::createShaderStages(std::vector<VkShaderModule> shaders, VkPipelineShaderStageCreateInfo &vertPipelineShaderStageCreateInfo,
                                       VkPipelineShaderStageCreateInfo &fragPipelineShaderStageCreateInfo) 
    {
        // 为了实际使用这些着色器，我们需要通过结构将它们分配到一个特定的流水线阶段
        // 首先填写顶点着色器
        vertPipelineShaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertPipelineShaderStageCreateInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vertPipelineShaderStageCreateInfo.module = shaders[0]; 
        vertPipelineShaderStageCreateInfo.pName  = "main";

        // 然后填写顶点着色器
        fragPipelineShaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragPipelineShaderStageCreateInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragPipelineShaderStageCreateInfo.module = shaders[1];
        fragPipelineShaderStageCreateInfo.pName  = "main";
    }

    bool VulkanAPI::createImage(uint32_t              imageWidth,
                                uint32_t              imageHeight,
                                VkFormat              format,
                                VkImageTiling         imageTiling,
                                VkImageUsageFlags     imageUsageFlags,
                                VkMemoryPropertyFlags memoryPropertyFlags,
                                VkImage&              image,
                                VkDeviceMemory&       memory,
                                VkImageCreateFlags    imageCreateFlags,
                                uint32_t              arrayLayers,
                                uint32_t              miplevels)
    {
        VkImageCreateInfo imageCreateInfo {};
        imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;  // sType是此结构的类型,VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO
        imageCreateInfo.flags         = imageCreateFlags;                     // flag是VkImageCreateFlagBits的位掩码，用于描述图像的其他参数
        imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;                     // imageType是VkImageType值，用于指定图像的基本尺寸。就图像类型而言，阵列纹理中的图层不算作尺寸，VK_IMAGE_TYPE_1D指定一维图像、VK_IMAGE_TYPE_2D指定二维图像、VK_IMAGE_TYPE_3D指定三维图像
        imageCreateInfo.extent.width  = imageWidth;                           // extent是一个VkExtent3D，它描述基本级别的每个维度中的数据元素数量
        imageCreateInfo.extent.height = imageHeight;
        imageCreateInfo.extent.depth  = 1;
        imageCreateInfo.mipLevels     = miplevels;                            // mipLevels描述可用于图像的最小采样的细节级别的数量
        imageCreateInfo.arrayLayers   = arrayLayers;                          // arrayLayers是图像中的层数
        imageCreateInfo.format        = format;                               // format是一种VkFormat，它描述了将包含在图像中的texel块的格式和类型       
        imageCreateInfo.tiling        = imageTiling;                          // tiling是一个VkImageTiling值，它指定内存中纹理元素块的平铺模式，
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;            // initialLayout是一个VkImageLayout值，它指定图像的所有图像子资源的初始VkImageLayout。
        imageCreateInfo.usage         = imageUsageFlags;                      // usage是VkImageUsageFlagBits的位掩码，用于描述图像的预期用法
        imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;                // samples是VkSampleCountFlagBits，用于指定每个纹理像素的样本数
        imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;            // SharingMode是VkSharingMode值，用于指定多个队列系列将访问图像时的图像共享模式

        if (vkCreateImage(mDevice, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
        {
            LOG_ERROR("failed to create image!");
            return;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(mDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(mPhysicalDevice, memRequirements.memoryTypeBits, memoryPropertyFlags);

        if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS)
        {
            LOG_ERROR("failed to allocate image memory!");
            return;
        }

        vkBindImageMemory(mDevice, image, memory, 0);
    }
        
    bool VulkanAPI::createImageView(VkImage&           image,
                                    VkFormat           format,
                                    VkImageAspectFlags imageAspectFlags,
                                    VkImageViewType    viewType,
                                    uint32_t           layoutCount,
                                    uint32_t           miplevels,
                                    VkImageView&       imageView)
    {
        VkImageViewCreateInfo imageViewCreateInfo {};
        imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image                           = image;
        imageViewCreateInfo.viewType                        = viewType;
        imageViewCreateInfo.format                          = format;
        imageViewCreateInfo.subresourceRange.aspectMask     = imageAspectFlags;
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.levelCount     = miplevels;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = layoutCount;

        if (vkCreateImageView(mDevice, &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            return imageView;
        }

        return imageView;
    }
            
    bool VulkanAPI::createFramebuffer(const FramebufferCreateInfo* pCreateInfo, VkFramebuffer &pFramebuffer)
    {
        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = (VkStructureType)pCreateInfo->sType;
        createInfo.pNext = (const void*)pCreateInfo->pNext;
        createInfo.flags = (VkFramebufferCreateFlags)pCreateInfo->flags;
        createInfo.renderPass = *pCreateInfo->renderPass;
        createInfo.attachmentCount = pCreateInfo->attachmentCount;
        createInfo.pAttachments = pCreateInfo->pAttachments.data();
        createInfo.width = pCreateInfo->width;
        createInfo.height = pCreateInfo->height;
        createInfo.layers = pCreateInfo->layers;

        VkResult result = vkCreateFramebuffer(mDevice, &createInfo, nullptr, &pFramebuffer);
        if (result == VK_SUCCESS)
        {
            return VK_SUCCESS;
        }
        else
        {
            LOG_ERROR("vkCreateFramebuffer failed!");
            return false;
        }
    }

    bool VulkanAPI::createGraphicsPipelines(const VkPipelineCache &pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo& pCreateInfos, VkPipeline &pPipelines)
    {
        VkResult result = vkCreateGraphicsPipelines(mDevice, pipelineCache, createInfoCount, &pCreateInfos, nullptr, &pPipelines);
        if (result == VK_SUCCESS)
        {
            return true;
        }
        else
        {
            LOG_ERROR("vkCreateGraphicsPipelines failed!");
            return false;
        }
    }

    bool VulkanAPI::createComputePipelines(VkPipelineCache &pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo pCreateInfos, VkPipeline &pPipelines)
    {
        VkResult result = vkCreateComputePipelines(mDevice, pipelineCache, createInfoCount, &pCreateInfos, nullptr, &pPipelines);
        if (result == VK_SUCCESS)
        {
            return VK_SUCCESS;
        }
        else
        {
            LOG_ERROR("vkCreateComputePipelines failed!");
            return false;
        }
    }
        
    bool VulkanAPI::createPipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, VkPipelineLayout &pipelineLayout)
    {
        if (vkCreatePipelineLayout(mDevice, pCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        return true;
    }
    
    bool VulkanAPI::createRenderPass(VkRenderPassCreateInfo renderpassCreateInfo, VkRenderPass &renderPass) 
    {   
        VkResult result = vkCreateRenderPass(mDevice, &renderpassCreateInfo, nullptr, &renderPass);
        if (result == VK_SUCCESS)
        {
            return VK_SUCCESS;
        }
        else
        {
            LOG_ERROR("vkCreateRenderPass failed!");
            return false;
        }
    }
    
    bool VulkanAPI::createCommandPool()
    {
        // default graphics command pool
        {
            // 命令池创建只需要两个参数
            VkCommandPoolCreateInfo commandPoolCreateInfo{};
            commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            commandPoolCreateInfo.pNext = NULL;
            // 每帧记录一个命令缓冲区，因此我们希望能够重置并重新记录它
            // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT：提示命令缓冲区经常用新命令重新记录（可能会改变内存分配行为）
            // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: 允许单独重新记录命令缓冲区，如果没有这个标志，它们都必须一起重置
            commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            commandPoolCreateInfo.queueFamilyIndex = mQueueIndices.graphicsFamily.value();

            if (vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mCommandPool) != VK_SUCCESS) 
            {
                LOG_ERROR("failed to create command pool!");
                return false;
            }
        }

        // other command pools
        {
            VkCommandPoolCreateInfo commandPoolCreateInfo;
            commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            commandPoolCreateInfo.pNext            = NULL;
            commandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            commandPoolCreateInfo.queueFamilyIndex = mQueueIndices.graphicsFamily.value();

            for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
            {
                if (vkCreateCommandPool(mDevice, &commandPoolCreateInfo, NULL, &mCommandPools[i]) != VK_SUCCESS)
                {
                    LOG_ERROR("failed to create command pool");
                    return false;
                }
            }
        }

        return true;
    }

    void VulkanAPI::resetCommandPool()
    {
        VkResult result = vkResetCommandPool(mDevice, mCommandPools[mCurrentFrameIndex], 0);
        if (VK_SUCCESS != result)
        {
            LOG_ERROR("failed to synchronize");
        }
    }

    void VulkanAPI::createCommandBuffers()
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
        commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1U;

        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            commandBufferAllocateInfo.commandPool = mCommandPools[i];
            VkCommandBuffer commandBuffer;
            if (vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
            {
                LOG_ERROR("vk allocate command buffers");
            }
            mCommandBuffers[i] = commandBuffer;
        }
    }
    
    bool VulkanAPI::createDescriptorPool() 
    {
        VkDescriptorPoolSize poolSizes[7];
        poolSizes[0].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        poolSizes[0].descriptorCount = 1 * COMMAND_BUFFER_POOL_MAX_SIZE;
        poolSizes[1].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSizes[1].descriptorCount = 2 * COMMAND_BUFFER_POOL_MAX_SIZE;
        poolSizes[2].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[2].descriptorCount = 5 * UNIFORM_BUFFER_POOL_MAX_SIZE;
        poolSizes[3].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[3].descriptorCount = 5 * UNIFORM_BUFFER_POOL_MAX_SIZE; // ImGui_ImplVulkan_CreateDeviceObjects
        poolSizes[4].type            = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        poolSizes[4].descriptorCount = 1 * UNIFORM_BUFFER_POOL_MAX_SIZE;
        poolSizes[5].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        poolSizes[5].descriptorCount = 1 * UNIFORM_BUFFER_POOL_MAX_SIZE;
        poolSizes[6].type            = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        poolSizes[6].descriptorCount = 1 * UNIFORM_BUFFER_POOL_MAX_SIZE;

        VkDescriptorPoolCreateInfo poolInfo {};
        poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
        poolInfo.pPoolSizes    = poolSizes;
        poolInfo.maxSets = 5 * UNIFORM_BUFFER_POOL_MAX_SIZE;
        poolInfo.flags = 0U;

        if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
        {
            LOG_ERROR("create descriptor pool");
            return false;
        }
        return true;
    }
    
    bool VulkanAPI::createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo &postProcessGlobalLayoutCreateInfo, VkDescriptorSetLayout &layout) 
    {
        VkResult result = vkCreateDescriptorSetLayout(mDevice, &postProcessGlobalLayoutCreateInfo, nullptr, &layout);
        if (result == VK_SUCCESS)
        {
            return VK_SUCCESS;
        }
        else
        {
            LOG_ERROR("vkCreateDescriptorSetLayout failed!");
            return false;
        }
    }

    bool VulkanAPI::createSyncPrimitives() 
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // the fence is initialized as signaled

        for (uint32_t i = 0; i < kMaxFramesInFlight; i++)
        {
            if (vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mImageAvailableForRenderSemaphores[i]) != VK_SUCCESS 
                || vkCreateSemaphore( mDevice, &semaphoreCreateInfo, nullptr, &mImageFinishedForPresentationSemaphores[i]) != VK_SUCCESS 
                || vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mIsFrameInFlightFences[i]) != VK_SUCCESS)
            {
                LOG_ERROR("vk create semaphore & fence");
            }
        }
    }
        
    void VulkanAPI::createVmaAllocator() 
    {
        // vma
        VmaVulkanFunctions vulkanFunctions    = {};
        vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr   = &vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocatorCreateInfo{};
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
        allocatorCreateInfo.physicalDevice = mPhysicalDevice;
        allocatorCreateInfo.device = mDevice;
        allocatorCreateInfo.instance = mInstance;
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

        if (vmaCreateAllocator(&allocatorCreateInfo, &mVmaAllocator) != VK_SUCCESS) 
        {
            throw std::runtime_error("vma create error!");
        }
    }

    bool VulkanAPI::allocateDescriptorSets(VkDescriptorSetAllocateInfo &allocateInfo, VkDescriptorSet &descriptorSet)
    {
         VkResult result = vkAllocateDescriptorSets(mDevice, &allocateInfo, &descriptorSet);
        if (result == VK_SUCCESS)
        {
            return true;
        }
        else
        {
            LOG_ERROR("vkAllocateDescriptorSets failed!");
            return false;
        }
    }

    bool VulkanAPI::allocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllacateInfo, VkCommandBuffer &pCommandBuffers)
    {   
        VkResult result = vkAllocateCommandBuffers(mDevice, pAllacateInfo, &pCommandBuffers);
        if (result == VK_SUCCESS)
        {
            return true;
        }
        else
        {
            LOG_ERROR("vkAllocateCommandBuffers failed!");
            return false;
        }
    }

    bool VulkanAPI::createFence(const VkFenceCreateInfo pCreateInfo, VkFence &pFence) 
    {
        VkResult result = vkCreateFence(mDevice, &pCreateInfo, nullptr, &pFence);
        if (result == VK_SUCCESS)
        {
            return true;
        }
        else
        {
            LOG_ERROR("vkCreateFence failed!");
            return false;
        }
    }
    
    bool VulkanAPI::waitForFences()
    {
        VkResult resWaitForFences = vkWaitForFences(mDevice, 1, &mIsFrameInFlightFences[mCurrentFrameIndex], VK_TRUE, UINT64_MAX);
        if (VK_SUCCESS != resWaitForFences)
        {
            LOG_ERROR("failed to synchronize!");
        }
    }
    
    bool VulkanAPI::waitForFences(uint32_t fenceCount, const VkFence* const* pFences, VkBool32 waitAll, uint64_t timeout)
    {
        //fence
        int fenceSize = fenceCount;
        std::vector<VkFence> fenceList(fenceSize);
        for (int i = 0; i < fenceSize; ++i)
        {
            fenceList[i] = *pFences[i];
        };

        VkResult result = vkWaitForFences(mDevice, fenceCount, fenceList.data(), waitAll, timeout);

        if (result == VK_SUCCESS)
        {
            return VK_SUCCESS;
        }
        else
        {
            LOG_ERROR("waitForFences failed");
            return false;
        }
    }

    bool VulkanAPI::resetFences(uint32_t fenceCount, std::vector<VkFence> fences)
    {
        VkResult result = vkResetFences(mDevice, fenceCount, fences.data());
        if (result == VK_SUCCESS)
        {
            return true;
        }
        else 
        {
            LOG_ERROR("vkReset Fences failed!");
            return false;
        }
    }

    bool VulkanAPI::createSemaphore(const VkSemaphoreCreateInfo pCreateInfo, VkSemaphore &pSemaphore)
    {   
        VkResult result = vkCreateSemaphore(mDevice, &pCreateInfo, nullptr, &pSemaphore);
        if (result == VK_SUCCESS)
        {
            return VK_SUCCESS;
        }
        else
        {
            LOG_ERROR("vkCreateSemaphore failed!");
            return false;
        }
    }
        
    bool VulkanAPI::createSampler(const VkSamplerCreateInfo pCreateInfo, VkSampler &pSampler)
    {
        VkResult result = vkCreateSampler(mDevice, &pCreateInfo, nullptr, &pSampler);
        if (result == VK_SUCCESS)
        {
            return VK_SUCCESS;
        }
        else
        {
            LOG_ERROR("vkCreateSampler failed!");
            return false;
        }
    }

    bool VulkanAPI::beginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
    {
        // 立即开始记录命令缓冲区
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = (VkStructureType)pBeginInfo->sType;
        commandBufferBeginInfo.pNext = (const void*)pBeginInfo->pNext;
        commandBufferBeginInfo.flags = (VkCommandBufferUsageFlags)pBeginInfo->flags;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

        VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

        if (result == VK_SUCCESS)
        {
            return true;
        }
        else
        {
            LOG_ERROR("vkBeginCommandBuffer failed!");
            return false;
        }
    }
        
    bool VulkanAPI::endCommandBuffer(VkCommandBuffer commandBuffer)
    {
        // 我们已经完成了命令缓冲区的记录：
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
        {
            LOG_ERROR("failed to record command buffer!");
            return false;
        }
        return true;
    }

    void VulkanAPI::cmdCopyImageToBuffer(
        VkCommandBuffer commandBuffer,
        VkImage srcImage,
        VkImageLayout srcImageLayout,
        VkBuffer &dstBuffer,
        uint32_t regionCount,
        std::vector<VkBufferImageCopy> pRegions)
    {
        vkCmdCopyImageToBuffer(commandBuffer, srcImage, (VkImageLayout)srcImageLayout, dstBuffer, regionCount, pRegions.data());
    }
    
    void VulkanAPI::cmdCopyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageAspectFlagBits srcFlag, VkImage &dstImage, VkImageAspectFlagBits dstFlag, uint32_t width, uint32_t height)
    {
        VkImageCopy imagecopyRegion = {};
        imagecopyRegion.srcSubresource = { (VkImageAspectFlags)srcFlag, 0, 0, 1 };
        imagecopyRegion.srcOffset = { 0, 0, 0 };
        imagecopyRegion.dstSubresource = { (VkImageAspectFlags)dstFlag, 0, 0, 1 };
        imagecopyRegion.dstOffset = { 0, 0, 0 };
        imagecopyRegion.extent = { width, height, 1 };

        vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imagecopyRegion);
    }

    bool VulkanAPI::cmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo &pRenderPassBegin, VkSubpassContents contents)
    {
        LOG_DEBUG("cmdBeginRenderPass");
        vkCmdBeginRenderPass(commandBuffer, &pRenderPassBegin, contents);
        return true;
    }
        
    bool VulkanAPI::cmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
    {
        vkCmdNextSubpass(commandBuffer, ((VkSubpassContents)contents));
        return true;
    }
    
    void VulkanAPI::cmdEndRenderPass(VkCommandBuffer commandBuffer)
    {
        return vkCmdEndRenderPass(commandBuffer);
    }

    void VulkanAPI::cmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) 
    {
        return vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    }

    void VulkanAPI::cmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport pViewports)
    {
        return vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, &pViewports);
    }
    
    void VulkanAPI::cmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D pScissors)
    {
        return vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, &pScissors);
    }

    void VulkanAPI::cmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
    {
        return vkCmdBindIndexBuffer(commandBuffer, buffer, (VkDeviceSize)offset, (VkIndexType)indexType);
    }

    void VulkanAPI::cmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, std::vector<VkBuffer> pBuffers, std::vector<VkDeviceSize> pOffsets)
    {
        return vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers.data(), pOffsets.data());
    }
    
    void VulkanAPI::cmdBindDescriptorSets(
        VkCommandBuffer commandBuffer,
        VkPipelineBindPoint pipelineBindPoint,
        VkPipelineLayout layout,
        uint32_t firstSet,
        uint32_t descriptorSetCount,
        std::vector<VkDescriptorSet> pDescriptorSets,
        uint32_t dynamicOffsetCount,
        std::vector<uint32_t> pDynamicOffsets)
    {
        return vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets.data(), dynamicOffsetCount, pDynamicOffsets.data());
    }
        
    void VulkanAPI::cmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
        return vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
    
    void VulkanAPI::cmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer &srcBuffer, VkBuffer &dstBuffer, uint32_t regionCount, VkBufferCopy pRegions)
    {
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, &pRegions);
    }
    
    void VulkanAPI::cmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    /* ---------------------------------- query ------------------------------------- */
    VkCommandPool VulkanAPI::getCommandPoor() const
    {
        return mCommandPool;
    }

    VkCommandBuffer VulkanAPI::getCurrentCommandBuffer() const{
        return mCurrentCommandBuffer;
    }

    VkQueue VulkanAPI::getGraphicsQueue() const
    {
        return mGraphicsQueue;
    }
    VkQueue VulkanAPI::getComputeQueue() const
    {
        return mComputeQueue;
    }

    SwapChainDesc VulkanAPI::getSwapchainInfo()
    {
        SwapChainDesc desc;
        desc.imageFormat = mSwapchainImageFormat;
        desc.extent = mSwapchainExtent;
        desc.viewport = mViewport;
        desc.scissor = mScissor;
        desc.imageViews = mSwapchainImageviews;
        return desc;
    }
        
    DepthImageDesc VulkanAPI::getDepthImageInfo() const
    {
        DepthImageDesc desc;
        desc.depthImageFormat = mDepthImageFormat;
        desc.depthImageView = mDepthImageView;
        desc.depthImage = mDepthImage;
        return desc;
    }

    uint8_t VulkanAPI::getCurrentFrameIndex()
    {
        return mCurrentFrameIndex;
    }
    
    void VulkanAPI::setCurrentFrameIndex(uint8_t index)
    {
        mCurrentFrameIndex = index;
    }
    
    uint8_t VulkanAPI::getMaxFramesInFlight() const
    {
        return kMaxFramesInFlight;
    }

    VkCommandBuffer VulkanAPI::beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool        = mCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }
        
    void VulkanAPI::endSingleTimeCommands(VkCommandBuffer &commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo {};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(mGraphicsQueue);

        vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
    }
        
    void VulkanAPI::pushEvent()
    {

    }
    
    void VulkanAPI::popEvent()
    {

    }

    void VulkanAPI::queueSubmit(VkQueue queue, uint32_t submitCount, std::vector<VkSubmitInfo> pSubmits, VkFence fence)
    {
        VkResult result = vkQueueSubmit(queue, submitCount, pSubmits.data(), fence);
        if (result == VK_SUCCESS)
        {

        }
        else
        {
            LOG_ERROR("vkQueueSubmit failed!");
        }
    }
    
    void VulkanAPI::queueWaitIdle(VkQueue queue)
    {
        VkResult result = vkQueueWaitIdle(queue);

        if (result == VK_SUCCESS)
        {

        }
        else
        {
            LOG_ERROR("vkQueueWaitIdle failed!");
        }
    }

    void VulkanAPI::prepareContext()
    {
        mCurrentCommandBuffer = mCommandBuffers[mCurrentFrameIndex];
    }

    bool VulkanAPI::prepareBeforePass()
    {   
        VkResult acquireImageResult = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, mImageAvailableForRenderSemaphores[mCurrentFrameIndex], VK_NULL_HANDLE, &mCurrentSwapchainImageIndex);
        
        if (VK_ERROR_OUT_OF_DATE_KHR == acquireImageResult)
        {
            recreateSwapchain();
            return VK_SUCCESS;
        }
        else if (VK_SUBOPTIMAL_KHR == acquireImageResult)
        {
            LOG_INFO("VK_SUBOPTIMAL_KHR");
            recreateSwapchain();

            // NULL submit to wait semaphore
            VkPipelineStageFlags  waitStages[] = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
            VkSubmitInfo          submitInfo   = {};
            submitInfo.sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount      = 1;
            submitInfo.pWaitSemaphores         = &mImageAvailableForRenderSemaphores[mCurrentFrameIndex];
            submitInfo.pWaitDstStageMask       = waitStages;
            submitInfo.commandBufferCount      = 0;
            submitInfo.pCommandBuffers         = NULL;
            submitInfo.signalSemaphoreCount    = 0;
            submitInfo.pSignalSemaphores       = NULL;
            
            VkResult   resResetFence           = vkResetFences(mDevice, 1, &mIsFrameInFlightFences[mCurrentFrameIndex]);
            if (VK_SUCCESS != resResetFence)
            {
                LOG_ERROR("vkResetFences failed!");
                return false;
            }

            VkResult resQueueSubmit            = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mIsFrameInFlightFences[mCurrentFrameIndex]);
            if(VK_SUCCESS != resQueueSubmit)
            {
                LOG_ERROR("vkQueueSubmit failed!");
            }

            mCurrentFrameIndex = (mCurrentFrameIndex + 1) % kMaxFramesInFlight;
            return VK_SUCCESS;
        }
        else 
        {
            if (VK_SUCCESS != acquireImageResult)
            {
                LOG_ERROR("vkAcquireNextImageKHR failed!");
                return false;
            }
        }

        // begin command buffer
        VkCommandBufferBeginInfo commandBufferBeginInfo {};
        commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags            = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

        VkResult resBeginCommandBuffer = vkBeginCommandBuffer(mCommandBuffers[mCurrentFrameIndex], &commandBufferBeginInfo);
        if (VK_SUCCESS != resBeginCommandBuffer)
        {
            LOG_ERROR("vkBeginCommandBuffer failed!");
            return false;
        }
        return true;
    }

    void VulkanAPI::submitAfterPass() 
    {
        // end command buffer
        VkResult resEndCommandBuffer = vkEndCommandBuffer(mCommandBuffers[mCurrentFrameIndex]);
        if (VK_SUCCESS != resEndCommandBuffer)
        {
            LOG_ERROR("vkEndCommandBuffer failed!");
            return;
        }

        VkSemaphore semaphores[1] = { mImageFinishedForPresentationSemaphores[mCurrentFrameIndex] };

        // submit command buffer
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo         submitInfo   = {};
        submitInfo.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount     = 1;
        submitInfo.pWaitSemaphores        = &mImageAvailableForRenderSemaphores[mCurrentFrameIndex];
        submitInfo.pWaitDstStageMask      = waitStages;
        submitInfo.commandBufferCount     = 1;
        submitInfo.pCommandBuffers        = &mCommandBuffers[mCurrentFrameIndex];
        submitInfo.signalSemaphoreCount   = 1;
        submitInfo.pSignalSemaphores      = semaphores;

        VkResult resResetFences = vkResetFences(mDevice, 1, &mIsFrameInFlightFences[mCurrentFrameIndex]);
        if (VK_SUCCESS != resResetFences)
        {
            LOG_ERROR("vkResetFences failed!");
            return;
        }

        VkResult resQueueSubmit = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mIsFrameInFlightFences[mCurrentFrameIndex]);
        if (VK_SUCCESS != resQueueSubmit)
        {
            LOG_ERROR("vkQueueSubmit failed!");
            return;
        }

        // present swapchain
        VkPresentInfoKHR presentInfo   = {};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = &mImageFinishedForPresentationSemaphores[mCurrentFrameIndex];
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &mSwapchain;
        presentInfo.pImageIndices      = &mCurrentSwapchainImageIndex;

        VkResult resPresent = vkQueuePresentKHR(mPresentQueue, &presentInfo);
        if (VK_ERROR_OUT_OF_DATE_KHR == resPresent || VK_SUBOPTIMAL_KHR == resPresent)
        {
            recreateSwapchain();
            // passUpdateAfterRecreateSwapchain();
        }
        else
        {
            if (VK_SUCCESS != resPresent)
            {
                LOG_ERROR("vkQueuePresentKHR failed!");
                return;
            }
        }

        mCurrentFrameIndex = (mCurrentFrameIndex + 1) % kMaxFramesInFlight;
    }

    /* ---------------------------------------- destory ---------------------------------------- */
    void VulkanAPI::clearSwapchain()
    {
        for (auto imageview : mSwapchainImageviews)
        {
            vkDestroyImageView(mDevice, imageview, NULL);
        }
        vkDestroySwapchainKHR(mDevice, mSwapchain, NULL); // also swapchain images
    }

    void VulkanAPI::destroyShaderModule(VkShaderModule &shaderModule)
    {
        vkDestroyShaderModule(mDevice, shaderModule, nullptr);

        delete(shaderModule);
    }
    
    void VulkanAPI::destroySemaphore(VkSemaphore semaphore)
    {
        vkDestroySemaphore(mDevice, semaphore, nullptr);
    }
        
    void VulkanAPI::destroyInstance()
    {
        vkDestroyInstance(mInstance, nullptr);
    }
    
    void VulkanAPI::destroyImageView(VkImageView imageView)
    {   
        vkDestroyImageView(mDevice, imageView, nullptr);
    }
    
    void VulkanAPI::destroyImage(VkImage image)
    {
        vkDestroyImage(mDevice, image, nullptr);
    }
    
    void VulkanAPI::destroyFramebuffer(VkFramebuffer framebuffer)
    {
        vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
    }
    
    void VulkanAPI::destroyFence(VkFence fence)
    {
        vkDestroyFence(mDevice, fence, nullptr);
    }
    
    void VulkanAPI::destroyDevice()
    {
        vkDestroyDevice(mDevice, nullptr);
    }
        
    void VulkanAPI::destroyCommandPool(VkCommandPool commandPool)
    {
        vkDestroyCommandPool(mDevice, commandPool, nullptr);
    }
    
    void VulkanAPI::destroyBuffer(VkBuffer &buffer)
    {
        vkDestroyBuffer(mDevice, buffer, nullptr);
    }

    void VulkanAPI::freeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount, VkCommandBuffer pCommandBuffers)
    {
        vkFreeCommandBuffers(mDevice, commandPool, commandBufferCount, &pCommandBuffers);
    }

    /* ---------------------------------------- memory ------------------------------------------ */ 
    void VulkanAPI::freeMemory(VkDeviceMemory &memory)
    {
        vkFreeMemory(mDevice, memory, nullptr);
    }
    
    bool VulkanAPI::mapMemory(VkDeviceMemory &memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
    {
        VkResult result = vkMapMemory(mDevice, memory, offset, size, flags, ppData);
        if (result == VK_SUCCESS)
        {
            return true;
        }
        else
        {
            LOG_ERROR("vkMapMemory failed!");
            return false;
        }
    }
    
    void VulkanAPI::unmapMemory(VkDeviceMemory &memory)
    {
        vkUnmapMemory(mDevice, memory);
    }

    bool VulkanAPI::checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : mValidationLayers) 
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) 
            {
                if (strcmp(layerName, layerProperties.layerName) == 0) 
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) 
            {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> VulkanAPI::getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        // extensions.insert(extensions.begin(), kRequiredInstanceExtensions.begin(), kRequiredInstanceExtensions.end());

        if (pEnableValidationLayers) 
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

#if defined(__MACH__)
        extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

        return extensions;
    }
        
    void  VulkanAPI::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo       = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }
        
    VkResult VulkanAPI::createDebugUtilsMessengerEXT(VkInstance instance,
                                                    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks*              pAllocator,
                                                    VkDebugUtilsMessengerEXT*                 pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } 
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }                                            
        
    void VulkanAPI::destroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                                  VkDebugUtilsMessengerEXT     debugMessenger,
                                                  const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            func(instance, debugMessenger, pAllocator);
        }
    }                                                           
        
    QueueFamilyIndices VulkanAPI::findQueueFamilies(VkPhysicalDevice physicalDevice)
    {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) 
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
            {
                indices.graphicsFamily = i;
            }

            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) // if support compute command queue
            {
                indices.computeFamily = i;
            }

            VkBool32 isPresentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, mSurface, &isPresentSupport);

            if (isPresentSupport) 
            {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) 
            {
                break;
            }

            i++;
        }

        return indices;
    }

    bool VulkanAPI::checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
    
    bool VulkanAPI::isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = findQueueFamilies(device);
        return indices.isComplete();
    }
    
    // 创建交换链还涉及到比创建实例和设备更多的设置，所以在我们能够继续之前，我们需要查询一些更多的细节。基本上有三种属性是我们需要检查的。
    SwapChainSupportDetails VulkanAPI::querySwapChainSupport(VkPhysicalDevice physicalDevice)
    {
        // 基本表面能力（交换链中图像的最小/最大数量，图像的最小/最大宽度和高度）
        // 表面格式（像素格式、色彩空间）
        // 可用的表现模式
        SwapChainSupportDetails details;

        // 表面能力开始。这些属性的查询很简单，并被返回到一个VkSurfaceCapabilitiesKHR结构中
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface, &details.capabilities);

        // 查询支持的表面格式。因为这是一个结构列表，它遵循熟悉的2个函数调用的仪式
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface, &formatCount, nullptr);

        if (formatCount != 0) 
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface, &formatCount, details.formats.data());
        }

        // 查询支持的演示模式
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mSurface, &presentModeCount, nullptr);

        if (presentModeCount != 0) 
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mSurface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }                                                     
        
    VkFormat VulkanAPI::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        LOG_ERROR("findSupportedFormat failed");
        return VkFormat();
    }
    
    VkFormat VulkanAPI::findDepthFormat()
    {
        return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
    
    VkSurfaceFormatKHR VulkanAPI::chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }
    
    VkPresentModeKHR VulkanAPI::chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes) 
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }
    
    VkExtent2D VulkanAPI::chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
        {
            return capabilities.currentExtent;
        } 
        else 
        {
            int width, height;
            glfwGetFramebufferSize(mWindow, &width, &height);

            VkExtent2D actualExtent = 
            {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
            // 在minImageExtent'和maxImageExtent’的范围内挑选与窗口最匹配的分辨率
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    uint32_t VulkanAPI::findMemoryType(VkPhysicalDevice      physical_device,
                                        uint32_t              type_filter,
                                        VkMemoryPropertyFlags properties_flag)
    {
        VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);
        for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
        {
            if (type_filter & (1 << i) &&
                (physical_device_memory_properties.memoryTypes[i].propertyFlags & properties_flag) == properties_flag)
            {
                return i;
            }
        }
        LOG_ERROR("findMemoryType error");
        return 0;
    }

    VkSampleCountFlagBits VulkanAPI::getMaxUsableSampleCount() 
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(mPhysicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; } 
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }
}