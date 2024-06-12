#include "VulkanContext.h"
#include "VulkanUtils.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wdeprecated-copy"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wunused-private-field"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wweak-vtables"
#pragma clang diagnostic ignored "-Wnullability-completeness"
#pragma clang diagnostic ignored "-Wextra-semi"
#pragma clang diagnostic ignored "-Wc++98-compat-extra-semi"

#define VMA_IMPLEMENTATION
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>


#define COMMAND_BUFFER_POOL_MAX_SIZE 128
#define UNIFORM_BUFFER_POOL_MAX_SIZE 128
// 12就有问题
// #define COMMAND_BUFFER_POOL_MAX_SIZE 12
// #define UNIFORM_BUFFER_POOL_MAX_SIZE 12

#define SEMAPHORE_MAX_SIZE 8


const std::vector<const char *> kValidationLayers = {
    "VK_LAYER_KHRONOS_validation",
};


const std::vector<const char *> kRequiredInstanceExtensions = {
#ifdef PLATFORM_OSX
    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
#endif
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
};


const std::vector<const char *> kRequiredDeviceExtensions = {
#ifdef PLATFORM_OSX
    VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
};


bool VulkanContext::create(GLFWwindow* window, bool debugOutput) {
    FUNCTION_TIMED("VulkanContext::create");
#define EXEC_VULKAN_STEP(step)                          \
    if (!step()) {                                      \
        printf("initVulkan error: %s failed", #step);   \
        return false;                                   \
    }

    window_ = window;
    debugOutput_ = debugOutput;
    // query instance extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> instanceExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, instanceExtensions.data());
    for (auto &ext : instanceExtensions) {
        instanceExtensions_[ext.extensionName] = ext;
    }

    EXEC_VULKAN_STEP(createInstance)
    EXEC_VULKAN_STEP(setupDebugMessenger)
    EXEC_VULKAN_STEP(createSurface)
    EXEC_VULKAN_STEP(pickPhysicalDevice)
    EXEC_VULKAN_STEP(createLogicalDevice)
    EXEC_VULKAN_STEP(createCommandPool)
    EXEC_VULKAN_STEP(createSwapchain)
    EXEC_VULKAN_STEP(createSwapchainImageViews)
    EXEC_VULKAN_STEP(createSyncObjects)

    commandBuffers_.reserve(COMMAND_BUFFER_POOL_MAX_SIZE);

    // vma
    VmaVulkanFunctions vulkanFunctions    = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr   = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorCreateInfo.physicalDevice = physicalDevice_;
    allocatorCreateInfo.device = device_;
    allocatorCreateInfo.instance = instance_;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    if (vmaCreateAllocator(&allocatorCreateInfo, &allocator_) != VK_SUCCESS) {
        throw std::runtime_error("vma create error!");
    }
    std:: cout << "vma create allocator" << std::endl;

    return true;
}


bool VulkanContext::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
}


void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
}


void VulkanContext::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}


VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}


QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}


bool VulkanContext::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    return indices.isComplete();
}


std::vector<const char*> VulkanContext::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}


bool VulkanContext::linearBlitAvailable(VkFormat format) {
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice_, format, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        return false;
    }
    return true;
}


bool VulkanContext::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
        return false;
    } 
    
    return true;
}


// 每个VkSurfaceFormatKHR'条目包含一个format’和一个colorSpace'成员。format成员指定了颜色通道和类型。
VkSurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}


// 演示模式可以说是交换链最重要的设置，因为它代表了向屏幕显示图像的实际条件
VkPresentModeKHR VulkanContext::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}


// 交换范围是交换链图像的分辨率，它几乎总是完全等于我们要绘制的窗口的分辨率*，以像素为单位
VkExtent2D VulkanContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window_, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        // 在minImageExtent'和maxImageExtent’的范围内挑选与窗口最匹配的分辨率
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}


// 创建交换链还涉及到比创建实例和设备更多的设置，所以在我们能够继续之前，我们需要查询一些更多的细节。基本上有三种属性是我们需要检查的。
SwapChainSupportDetails VulkanContext::querySwapChainSupport(VkPhysicalDevice device) {
        // 基本表面能力（交换链中图像的最小/最大数量，图像的最小/最大宽度和高度）
        // 表面格式（像素格式、色彩空间）
        // 可用的表现模式
        SwapChainSupportDetails details;

        // 表面能力开始。这些属性的查询很简单，并被返回到一个VkSurfaceCapabilitiesKHR结构中
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

        // 查询支持的表面格式。因为这是一个结构列表，它遵循熟悉的2个函数调用的仪式
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
        }

        // 查询支持的演示模式
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
        }

        return details;
}


bool VulkanContext::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
        return false;
    }
    return true;
}


bool VulkanContext::createSurface(){
    if (glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) != VK_SUCCESS){
        std::runtime_error("glfwCreateWindowSurface failed!");
        return false;
    }
    return true;
}


bool VulkanContext::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
    for (const auto& device : devices) {
        QueueFamilyIndices indices = findQueueFamilies(device);
        if (indices.isComplete()) {
            queueIndices_ = indices;
            physicalDevice_ = device;
            break;
        }
    }

    if (physicalDevice_ == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
        return false;
    } else {
        vkGetPhysicalDeviceProperties(physicalDevice_, &deviceProperties_);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &deviceMemoryProperties_);

        // query device extensions
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, deviceExtensions.data());
        for (auto &ext : deviceExtensions) {
            deviceExtensions_[ext.extensionName] = ext;
        }
    }
    
    return physicalDevice_ != VK_NULL_HANDLE;
}


bool VulkanContext::createLogicalDevice() {
    std:: cout << "create logical device-------------------------------------------------------------" << std::endl;
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
        return false;
    }

    vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
    return true;
}


bool VulkanContext::createSwapchain() {
    std::cout << "create swap chain" << std::endl;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice_);

    // 如果满足了swapChainAdequate的条件，那么支持肯定是足够的，但是仍然可能有许多不同的模式，有不同的优化。我们现在要写几个函数来找到最佳交换链的正确设置。有三种类型的设置需要确定。
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities); // 看看这个有影响吗

    // 除了这些属性之外，我们还必须决定我们希望在交换链中拥有多少图像
    // 我们建议至少要比最小值多请求一个图像。
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    //不要超过最大的图片数量，其中0是一个特殊的值，意味着没有最大的数量。
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // 创建交换链对象需要填写一个大的结构
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    // 每个图像所包含的层数。除非你正在开发一个立体的3D应用程序，否则这总是1
    createInfo.imageArrayLayers = 1;
    // 我们将在交换链中使用图像的哪种操作
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    // vk_sharing_mode_exclusive。一个图像一次由一个队列家族拥有，在另一个队列家族中使用它之前，必须明确转移所有权。这个选项提供了最好的性能。
    // vk_sharing_mode_concurrent。图像可以在多个队列家族中使用，无需明确的所有权转移。
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // 我们可以指定在交换链中，如果支持某种变换（capabilities中的supportedTransforms
    // 要指定你不想要任何变换，只需指定当前的变换。
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // 是否应该使用alpha通道与窗口系统中的其他窗口进行混合
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
        return false;
    }

    // 在createSwapChain'函数的末尾添加了检索句柄的代码
    // 我们只指定了交换链中图像的最低数量，所以实现允许创建一个有更多图像的交换链。
    // 先用vkGetSwapchainImagesKHR查询最终的图像数量，然后调整容器的大小，最后再调用它来检索手柄。
    vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
    swapChainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages_.data());

    // 将我们为交换链图像选择的格式和范围存储在成员变量中。
    swapChainImageFormat_ = surfaceFormat.format;
    swapChainExtent_ = extent;
    return true;
}


// 创建createImageViews函数，并在交换链创建后立即调用它。
bool VulkanContext::createSwapchainImageViews() {
    std::cout << "create swapchain imageviews" << std::endl;
    // 调整列表的大小，以适应我们将要创建的所有图像视图。
    swapChainImageViews_.resize(swapChainImages_.size());

    // 设置循环，在所有交换链图像上进行迭代。
    for (size_t i = 0; i < swapChainImages_.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        // 创建图像视图的参数在VkImageViewCreateInfo结构中指定。前面的几个参数是直接的。
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages_[i];
        // viewType 和 format字段指定了图像数据的解释方式。
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat_;
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

        if (vkCreateImageView(device_, &createInfo, nullptr, &swapChainImageViews_[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
    return true;
}


bool VulkanContext::createSyncObjects() {
    // 创建信号量
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // 创建围栏需要填写VkFenceCreateInfo
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // 在信号状态下创建围栏，以便第一次调用 vkWaitForFences()立即返回，因为围栏已经发出信号。
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &imageAvailableSemaphore_) != VK_SUCCESS ||
        vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &renderFinishedSemaphore_) != VK_SUCCESS ||
        vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFence_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
    return true;
}

bool VulkanContext::createCommandPool() {
    std::cout << "create command pool" << std::endl;
    // 命令池创建只需要两个参数
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice_);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // 每帧记录一个命令缓冲区，因此我们希望能够重置并重新记录它
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT：提示命令缓冲区经常用新命令重新记录（可能会改变内存分配行为）
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: 允许单独重新记录命令缓冲区，如果没有这个标志，它们都必须一起重置
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
        return false;
    }
    return true;
}


void VulkanContext::createGPUBuffer(AllocatedBuffer &buffer, VkDeviceSize size, VkBufferUsageFlags usage) {
    std::cout << "start create GPU Buffer" << std::endl;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkResult result = vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.allocation, &buffer.allocInfo);
    std::cout << "end create GPU Buffer" << std::endl;
    // VK_CHECK(vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.allocation, &buffer.allocInfo));
}


void VulkanContext::createUniformBuffer(AllocatedBuffer &buffer, VkDeviceSize size) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VK_CHECK(vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.allocation, &buffer.allocInfo));
}


void VulkanContext::createStagingBuffer(AllocatedBuffer &buffer, VkDeviceSize size) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  // 从CPU到GPU
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VK_CHECK(vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.allocation, &buffer.allocInfo));
}   


bool VulkanContext::createImageMemory(AllocatedImage &image, uint32_t properties, const void *pNext) {
    if (image.memory != VK_NULL_HANDLE) {
        return true;
    }

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device_, image.image, &memReqs);
    image.allocationSize = memReqs.size;

    VkMemoryAllocateInfo memAllocaInfo{};
    memAllocaInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocaInfo.pNext = pNext;
    memAllocaInfo.allocationSize = memReqs.size;
    memAllocaInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, properties);
    if (memAllocaInfo.memoryTypeIndex == VK_MAX_MEMORY_TYPES) {
        throw std::runtime_error("vulkan memory type not available, property flags");
        return;
    }

    if (VK_SUCCESS != vkAllocateMemory(device_, &memAllocaInfo, nullptr, &image.memory)) {
        throw std::runtime_error("vulkan allocate memory error!");
        return;
    }
    if (VK_SUCCESS != vkBindImageMemory(device_, image.image, image.memory, 0)) {
        throw std::runtime_error("vulkan bind image memory error!");
        return;
    }
}

UniformBuffer *VulkanContext::getNewUniformBuffer(VkDeviceSize size) {
    auto it = uniformBufferPool_.find(size);
    if (it == uniformBufferPool_.end()) {
        std::cout << "uniformBufferPool_" << size << std::endl;
        std::vector<UniformBuffer> uboPool;
        // reserver函数用来给vector预分配存储区大小，即capacity的值 ，但是没有给这段内存进行初始化
        uboPool.reserve(UNIFORM_BUFFER_POOL_MAX_SIZE);
        uniformBufferPool_[size] = std::move(uboPool);
    }
    auto &pool = uniformBufferPool_[size];
    for (auto &buff : pool) {
        if (!buff.inUse) {
            std::cout << "uniformBufferPool_ inUse" << size << std::endl;
            buff.inUse = true;
            return &buff;
        }
    }

    UniformBuffer buff{};
    buff.inUse = true;
    createUniformBuffer(buff.buffer, size);
    buff.mapPtr = buff.buffer.allocInfo.pMappedData;
    pool.push_back(buff);
    maxUniformBufferPoolSize_ = std::max(maxUniformBufferPoolSize_, pool.size());
    std::cout << "maxUniformBufferPoolSize" << maxUniformBufferPoolSize_ << std::endl;
    if (maxUniformBufferPoolSize_ >= UNIFORM_BUFFER_POOL_MAX_SIZE) {
        // throw std::runtime_error("error: uniform buffer pool size exceed");
        std::cout << "error: uniform buffer pool size exceed" << std::endl;
        return;
    } 
    return &pool.back();
}


uint32_t VulkanContext::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < deviceMemoryProperties_.memoryTypeCount; i++) {
        if ((typeBits & 1) == 1) {
            if ((deviceMemoryProperties_.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        typeBits >>= 1;
    }
    return VK_MAX_MEMORY_TYPES;
}


CommandBuffer *VulkanContext::beginCommands() {
    auto *commandBuffer = getNewCommandBuffer();

    // 立即开始记录命令缓冲区
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer->cmdBuffer, &beginInfo);

    return commandBuffer;
}


void VulkanContext::endCommands(CommandBuffer *commandBuffer, 
                                const std::vector<VkSemaphore> &waitSemaphores,
                                const std::vector<VkSemaphore> &signalSemaphores) {
    // 我们已经完成了命令缓冲区的记录：
    if (vkEndCommandBuffer(commandBuffer->cmdBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // 指定实际提交执行的命令缓冲区
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->cmdBuffer;

    if (!waitSemaphores.empty()) {
        if (waitSemaphores.size() > SEMAPHORE_MAX_SIZE) {
            std::runtime_error("endCommands error: wait semaphores max size exceeded");
        }
        // 指定在执行开始之前要等待的信号量以及要等待的管道阶段
        static std::vector<VkPipelineStageFlags> waitStageMasks(SEMAPHORE_MAX_SIZE, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStageMasks.data();
    } else {
        submitInfo.waitSemaphoreCount = 0;
    }

    if (!signalSemaphores.empty()) {
        // 指定在命令缓冲区执行完毕后发送信号的信号量
        submitInfo.signalSemaphoreCount = signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();
    } else {
        submitInfo.signalSemaphoreCount = 0;
    }
    // 使用 vkQueueSubmit 将命令缓冲区提交到图形队列 
    if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, commandBuffer->fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    purgeCommandBuffers();
}


void VulkanContext::waitCommands(CommandBuffer *commandBuffer) {
    // 等待所有的栅栏，但在单个栅栏的情况下，这无关紧要
    vkWaitForFences(device_, 1, &commandBuffer->fence, VK_TRUE, UINT64_MAX);
}


void VulkanContext::allocateCommandBuffer(VkCommandBuffer &cmdBuffer) {
    VkCommandBufferAllocateInfo allocInfo{};
    // 内存传输操作是使用命令缓冲区执行的，就像绘图命令一样。因此我们必须首先分配一个临时命令缓冲区。
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool_; // 短期缓冲区创建一个单独的命令池
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device_, &allocInfo, &cmdBuffer);
}


CommandBuffer* VulkanContext::getNewCommandBuffer() {
    for(auto  &cmd : commandBuffers_) {
        if (!cmd.inUse) {
            if (cmd.cmdBuffer == VK_NULL_HANDLE) {
                allocateCommandBuffer(cmd.cmdBuffer);
            }
            cmd.inUse = true;
            return &cmd;
        }
    }

    CommandBuffer cmd{};
    cmd.inUse = true;
    allocateCommandBuffer(cmd.cmdBuffer);

    // 创建信号量
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &cmd.semaphore) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization semaphore for a frame!");
    }

    // 创建围栏需要填写VkFenceCreateInfo
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // 在信号状态下创建围栏，以便第一次调用 vkWaitForFences()立即返回，因为围栏已经发出信号。
    // fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(device_, &fenceInfo, nullptr, &cmd.fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization fence for a frame!");
    }

    commandBuffers_.push_back(cmd);
    maxCommandBufferPoolSize_ = std::max(maxCommandBufferPoolSize_, commandBuffers_.size());
    if (maxCommandBufferPoolSize_ >= COMMAND_BUFFER_POOL_MAX_SIZE) {
        std::runtime_error("error: command buffer pool size exceed");
    }
    return &commandBuffers_.back();
}


void VulkanContext::purgeCommandBuffers() {
    for (auto &cmd : commandBuffers_) {
        if (!cmd.inUse) {
            continue;
        }
        VkResult waitRet = vkGetFenceStatus(device_, cmd.fence);
        if (waitRet == VK_SUCCESS) {
            // reset command buffer
            vkFreeCommandBuffers(device_, commandPool_, 1, &cmd.cmdBuffer);
            cmd.cmdBuffer = VK_NULL_HANDLE;
            
            // reset fence
            vkResetFences(device_, 1, &cmd.fence);

            // reset uniform buffers
            for (auto *buff : cmd.uniformBuffers) {
                buff->inUse = false;
            }
            cmd.uniformBuffers.clear();

            // reset descriptor sets
            for (auto *set : cmd.descriptorSets) {
                set->inUse = false;
            }
            cmd.descriptorSets.clear();

            // reset flag
            cmd.inUse = false;
        }
    }
}


void VulkanContext::beforeDrawFrame() {
    // 等待所有的栅栏，但在单个栅栏的情况下，这无关紧要
    // vkWaitForFences(device_, 1, &inFlightFence_, VK_TRUE, UINT64_MAX);

    // uint32_t imageIndex;
    // 希望从中获取图像的逻辑设备和交换链
    // 指定图像可用的超时时间（以纳秒为单位）：使用 64 位无符号整数的最大值意味着我们可以有效地禁用超时。
    // 开始绘制的时间点，最后一个参数指定一个变量来输出已变为可用的交换链图像的索引
    VkResult result = vkAcquireNextImageKHR(device_, swapChain_, UINT64_MAX, imageAvailableSemaphore_, VK_NULL_HANDLE, &imageIndex_);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // vkResetFences(device_, 1, &inFlightFence_);
}


void VulkanContext::afterDrawFrame(VkCommandBuffer &drawCmd) {
    std::cout << "VulkanContext::afterDrawFrame" << std::endl;
    // VkSubmitInfo submitInfo{};
    // submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // // 指定在执行开始之前要等待的信号量以及要等待的管道阶段
    // VkSemaphore waitSemaphores[] = {imageAvailableSemaphore_};
    // VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    // // 指定了写入颜色附件的图形管道阶段
    // submitInfo.waitSemaphoreCount = 1;
    // submitInfo.pWaitSemaphores = waitSemaphores;
    // submitInfo.pWaitDstStageMask = waitStages;
    // // 指定实际提交执行的命令缓冲区
    // submitInfo.commandBufferCount = 1;
    // submitInfo.pCommandBuffers = &drawCmd;

    // 指定在命令缓冲区执行完毕后发送信号的信号量signalSemaphoreCount
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore_};
    // submitInfo.signalSemaphoreCount = 1;
    // submitInfo.pSignalSemaphores = signalSemaphores;

    // // 使用 vkQueueSubmit 将命令缓冲区提交到图形队列 
    // if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFence_) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to submit draw command buffer!");
    // }

    // 绘制框架的最后一步是将结果提交回交换链，使其最终显示在屏幕上。
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // 发生之前要等待的信号量
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    // 获取将发出信号的信号量并等待它们
    // 呈现图像的交换链以及每个交换链的图像索引
    VkSwapchainKHR swapChains[] = {swapChain_};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    // 向交换链提交呈现图像的请求
    presentInfo.pImageIndices = &imageIndex_;

    VkResult result = vkQueuePresentKHR(presentQueue_, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}


void VulkanContext::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window_, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window_, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device_);

    cleanupSwapChain();
    createSwapchain();
    createSwapchainImageViews();
}


void VulkanContext::cleanupSwapChain() {
    for (auto imageView : swapChainImageViews_) {
        vkDestroyImageView(device_, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device_, swapChain_, nullptr);
}


void VulkanContext::destroy() {
    cleanupSwapChain();

    for (auto &kv : uniformBufferPool_) {
        for (auto &buff : kv.second) {
            buff.buffer.destroy(allocator_);
            buff.mapPtr = nullptr;
            buff.inUse = false;
        }
    }

    for (auto &cmd : commandBuffers_) {
        vkDestroyFence(device_, cmd.fence, nullptr);
        vkDestroySemaphore(device_, cmd.semaphore, nullptr);
        if (cmd.cmdBuffer != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(device_, commandPool_, 1, &cmd.cmdBuffer);
        }
    }
    vkDestroyCommandPool(device_, commandPool_, nullptr);

    // vma
    vmaDestroyAllocator(allocator_);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
    }
    // device & instance
    vkDestroyDevice(device_, nullptr);
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);
}

