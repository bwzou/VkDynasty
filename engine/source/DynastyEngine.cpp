
#include "DynastyEngine.h"


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

void DynastyEngine::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void DynastyEngine::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    DynastyEngine::populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

std::vector<const char*> DynastyEngine::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool DynastyEngine::checkValidationLayerSupport() {
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

VKAPI_ATTR VkBool32 VKAPI_CALL DynastyEngine::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
QueueFamilyIndices DynastyEngine::findQueueFamilies(VkPhysicalDevice device) {
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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

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

bool DynastyEngine::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = DynastyEngine::findQueueFamilies(device);

    return indices.isComplete();
}

// 每个VkSurfaceFormatKHR'条目包含一个format’和一个colorSpace'成员。format成员指定了颜色通道和类型。
VkSurfaceFormatKHR DynastyEngine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

// 演示模式可以说是交换链最重要的设置，因为它代表了向屏幕显示图像的实际条件
VkPresentModeKHR DynastyEngine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

// 交换范围是交换链图像的分辨率，它几乎总是完全等于我们要绘制的窗口的分辨率*，以像素为单位
VkExtent2D DynastyEngine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

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
SwapChainSupportDetails DynastyEngine::querySwapChainSupport(VkPhysicalDevice device) {
        // 基本表面能力（交换链中图像的最小/最大数量，图像的最小/最大宽度和高度）
        // 表面格式（像素格式、色彩空间）
        // 可用的表现模式
        SwapChainSupportDetails details;

        // 表面能力开始。这些属性的查询很简单，并被返回到一个VkSurfaceCapabilitiesKHR结构中
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        // 查询支持的表面格式。因为这是一个结构列表，它遵循熟悉的2个函数调用的仪式
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        // 查询支持的演示模式
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
}

void DynastyEngine::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, nullptr);
}

void DynastyEngine::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}

void DynastyEngine::initVulkan() {
    DynastyEngine::createInstance();
    DynastyEngine::setupDebugMessenger();
    DynastyEngine::createSurface();
    DynastyEngine::pickPhysicalDevice();
    DynastyEngine::createLogicalDevice();     
    DynastyEngine::createSwapChain();
    DynastyEngine::createImageViews();
    DynastyEngine::createRenderPass();
    DynastyEngine::createDescriptorSetLayout();
    DynastyEngine::createGraphicsPipeline();
    DynastyEngine::createCommandPool();
    DynastyEngine::createFramebuffers();

    DynastyEngine::createVertexBuffer();
    DynastyEngine::createIndexBuffer();
    DynastyEngine::createUniformBuffer();
    DynastyEngine::createDescriptorPool();
    DynastyEngine::createDescriptorSets();
    DynastyEngine::createCommandBuffer();
    DynastyEngine::createSyncObjects();
}

void DynastyEngine::createInstance() {
    if (enableValidationLayers && !DynastyEngine::checkValidationLayerSupport()) {
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

    auto extensions = DynastyEngine::getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        DynastyEngine::populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void DynastyEngine::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void DynastyEngine::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (DynastyEngine::isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void DynastyEngine::createLogicalDevice() {
    QueueFamilyIndices indices = DynastyEngine::findQueueFamilies(physicalDevice);

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

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void DynastyEngine::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = DynastyEngine::querySwapChainSupport(physicalDevice);

    // 如果满足了swapChainAdequate的条件，那么支持肯定是足够的，但是仍然可能有许多不同的模式，有不同的优化。我们现在要写几个函数来找到最佳交换链的正确设置。有三种类型的设置需要确定。
    VkSurfaceFormatKHR surfaceFormat = DynastyEngine::chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = DynastyEngine::chooseSwapPresentMode(swapChainSupport.presentModes);
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
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    // 每个图像所包含的层数。除非你正在开发一个立体的3D应用程序，否则这总是1
    createInfo.imageArrayLayers = 1;
    // 我们将在交换链中使用图像的哪种操作
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
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

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    // 在createSwapChain'函数的末尾添加了检索句柄的代码
    // 我们只指定了交换链中图像的最低数量，所以实现允许创建一个有更多图像的交换链。
    // 先用vkGetSwapchainImagesKHR查询最终的图像数量，然后调整容器的大小，最后再调用它来检索手柄。
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    // 将我们为交换链图像选择的格式和范围存储在成员变量中。
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

// 创建createImageViews函数，并在交换链创建后立即调用它。
void DynastyEngine::createImageViews() {
    // 调整列表的大小，以适应我们将要创建的所有图像视图。
    swapChainImageViews.resize(swapChainImages.size());

    // 设置循环，在所有交换链图像上进行迭代。
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        // 创建图像视图的参数在VkImageViewCreateInfo结构中指定。前面的几个参数是直接的。
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        // viewType 和 format字段指定了图像数据的解释方式。
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        // components字段允许你对颜色通道进行旋转,我们将坚持使用默认的映射。
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // ubresourceRange字段描述了图像的目的是什么，应该访问图像的哪一部分。
        // 我们的图像将被用作颜色目标，没有任何mipmapping级别或多个层次。
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

// 一个渲染通道可以由多个子通道组成。子通道是依赖于之前通道中帧缓冲区内容的后续渲染操作，例如，一连串的后期处理效果被相继应用。
void DynastyEngine::createRenderPass() {
    // 只有一个单一的颜色缓冲区附件，由交换链中的一个图像代表
    VkAttachmentDescription colorAttachment{};
    // 颜色附件的format应该与交换链图像的格式相匹配
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // 这里采用存储操作。
    // stencilLoadOp / stencilStoreOp 适用于模板数据
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // attachment参数通过附件描述数组中的索引指定要引用的附件
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    // 子通道
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // 明确指出这是一个图形子通道
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;


    // 渲染通道中的子通道会自动处理图像布局转换。这些转换由subpass dependencies控制，它指定子通道之间的内存和执行依赖性。
    // 我们现在只有一个子通道，但是这个子通道之前和之后的操作也算作隐式“子通道”。
    VkSubpassDependency dependency{};
    // 两个字段指定依赖项和依赖子通道的索引
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    // 指定要等待的操作以及这些操作发生的阶段
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    // 需要等待交换链完成从图像中的读取，然后才能访问它
    // 等待的操作在颜色附件阶段，涉及颜色附件的写入
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void DynastyEngine::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    // 前两个字段指定binding在着色器中使用的和描述符的类型，它是一个统一的缓冲区对象
    uboLayoutBinding.binding = 0;
    // 指定数组中值的数量
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // 该pImmutableSamplers字段仅与图像采样相关的描述符相关
    uboLayoutBinding.pImmutableSamplers = nullptr;
    // 我们的 MVP 转换是在一个单一的统一缓冲对象中
    // 指定在哪个着色器阶段将引用描述符。该字段可以是值或值stageFlags的组合。
    // 在我们的例子中，我们只引用来自顶点着色器的描述符
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // 我们的 MVP 转换是在一个单一的统一缓冲对象中
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DynastyEngine::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // 描述符集将包含哪些描述符类型以及它们的数量
    // poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].descriptorCount = 1;
    
    // 我们将为每一帧分配这些描述符之一。此池大小结构由 main 引用VkDescriptorPoolCreateInfo
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    // poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

// 添加一个新的类成员来存储描述符池的句柄并调用 vkCreateDescriptorPool创建它
void DynastyEngine::createDescriptorSets() {
    // std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    std::vector<VkDescriptorSetLayout> layouts(1, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    // allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts.data();

    // descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    descriptorSets.resize(1);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    //     VkDescriptorBufferInfo bufferInfo{};
    //     bufferInfo.buffer = uniformBuffers[i];
    //     bufferInfo.offset = 0;
    //     bufferInfo.range = sizeof(UniformBufferObject);

    //     VkDescriptorImageInfo imageInfo{};
    //     imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //     imageInfo.imageView = textureImageView;
    //     imageInfo.sampler = textureSampler;

    //     std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    //     descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //     descriptorWrites[0].dstSet = descriptorSets[i];
    //     descriptorWrites[0].dstBinding = 0;
    //     descriptorWrites[0].dstArrayElement = 0;
    //     descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //     descriptorWrites[0].descriptorCount = 1;
    //     descriptorWrites[0].pBufferInfo = &bufferInfo;

    //     descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //     descriptorWrites[1].dstSet = descriptorSets[i];
    //     descriptorWrites[1].dstBinding = 1;
    //     descriptorWrites[1].dstArrayElement = 0;
    //     descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //     descriptorWrites[1].descriptorCount = 1;
    //     descriptorWrites[1].pImageInfo = &imageInfo;

    //     vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    // }

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        // VkDescriptorImageInfo imageInfo{};
        // imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // imageInfo.imageView = textureImageView;
        // imageInfo.sampler = textureSampler;

        VkWriteDescriptorSet descriptorWrite{};
        // std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        // 前两个字段指定要更新的描述符集和绑定。
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[0];
        // 统一的缓冲区绑定索引0
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        // 指定描述符的类型。可以一次更新数组中的多个描述符
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        // 该pBufferInfo字段用于引用缓冲区数据的描述符
        descriptorWrite.pBufferInfo = &bufferInfo;

        // descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // descriptorWrites[1].dstSet = descriptorSets[0];
        // descriptorWrites[1].dstBinding = 1;
        // descriptorWrites[1].dstArrayElement = 0;
        // descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // descriptorWrites[1].descriptorCount = 1;
        // descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void DynastyEngine::createGraphicsPipeline() {
    auto vertShaderCode = DynastyEngine::readFile("/Users/bowenzou/Workspace/Documents/games-vulkan/dynasty/engine/shader/generated/spv/shader_base.vert.spv");
    auto fragShaderCode = DynastyEngine::readFile("/Users/bowenzou/Workspace/Documents/games-vulkan/dynasty/engine/shader/generated/spv/shader_base.frag.spv");
    
    VkShaderModule vertShaderModule = DynastyEngine::createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = DynastyEngine::createShaderModule(fragShaderCode);

    // 为了实际使用这些着色器，我们需要通过结构将它们分配到一个特定的流水线阶段
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    // 首先填写顶点着色器
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    // 然后填写顶点着色器
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    // 定义一个包含这两个结构的数组，以后我们将在实际的管道创建步骤中用来引用它们。
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // 在Vulkan中，你必须明确从视口大小到颜色混合功能的一切。
    // 描述了将传递给顶点着色器的顶点数据的格式。它大致以两种方式描述。
    // 绑定：数据之间的间距以及数据是逐顶点还是逐实例（参见 实例）
    // 属性描述：传递给顶点着色器的属性的类型，从哪个绑定加载它们以及在哪个偏移量
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 将这个视口和剪切形矩形组合成一个视口状态
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // 光栅化从顶点着色器中获取由顶点塑造的几何体，并将其转化为片段，由片段着色器进行着色。
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // depthClampEnable被设置为VK_TRUE，那么超出近平面和远平面的片段会被夹住，而不是丢弃
    // 使用这个需要启用一个GPU功能。
    rasterizer.depthClampEnable = VK_FALSE;
    // 如果rasterizerDiscardEnable被设置为VK_TRUE，那么几何体就不会通过光栅化器阶段。
    // 这基本上是禁用任何输出到帧缓冲区的功能。
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // polygonMode 决定了如何生成几何体的片段。有以下模式可供选择。
    // VK_POLYGON_MODE_FILL：用片段填充多边形的区域。
    // VK_POLYGON_MODE_LINE：多边形的边缘被绘制成线。
    // VK_POLYGON_MODE_POINT：多边形顶点被画成点。
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    // 它以片段的数量来描述线条的厚度。
    // 任何比1.0f厚的线都需要你启用wideLinesGPU功能。
    rasterizer.lineWidth = 1.0f;
    // cullMode 变量确定要使用的面剔除类型。您可以禁用剔除、剔除正面、剔除背面或两者。
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    // frontFace 变量指定被视为正面的面的顶点顺序，可以是顺时针或逆时针。
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    // 通过添加一个常量值或根据片段的斜率对其进行偏置来改变深度值
    rasterizer.depthBiasEnable = VK_FALSE;

    // VkPipelineMultisampleStateCreateInfo结构配置了多重采样，这是执行抗锯齿的方法之一
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // 在片段着色器返回颜色后，需要将其与已经存在于帧缓冲器中的颜色结合起来。这种转换被称为颜色混合，有两种方法可以做到这一点。
    // 将新旧值混合，产生最终的颜色
    // 使用位操作将新旧值结合起来

    // 每个附加帧缓冲区的配置
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    // 第二个结构引用了所有帧缓冲器的结构数组，并允许你设置混合常数，你可以在上述计算中作为混合系数使用。
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // 我们在前面的结构中指定的有限数量的状态可以在不重新创建管道的情况下实际改变。例如，视口的大小、线宽和混合常数。如果你想这样做，那么你必须填写一个VkPipelineDynamicStateCreateInfo结构，
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // 我们需要在管道创建期间指定描述符集布局，以告知 Vulkan 着色器将使用哪些描述符。描述符集布局在管道布局对象中指定。修改VkPipelineLayoutCreateInfo 以引用布局对象
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    // pipelineLayoutInfo.setLayoutCount = 0;
    // pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // 你可以在着色器中使用uniform值，它是类似于动态状态变量的球状物，可以在绘制时改变，以改变着色器的行为，而不必重新创建它们。它们通常被用来向顶点着色器传递变换矩阵，或者在片段着色器中创建纹理采样器。
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    // 引用VkPipelineShaderStageCreateInfo结构的阵列。
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    // 引用所有描述固定功能阶段的结构。
    // 之后是管道布局，它是一个Vulkan句柄而不是一个结构指针。
    pipelineInfo.layout = pipelineLayout;
    // 渲染通道的引用和子通道的索引。
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    // Vulkan允许你通过衍生现有的管道来创建一个新的图形管道
    // 可以用basePipelineHandle指定一个现有管道的句柄
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    // 第二个参数，我们为其传递了VK_NULL_HANDLE参数，引用了一个可选的VkPipelineCache对象。管线缓存可以用来存储和重用与管线创建相关的数据，
    // 跨越对vkCreateGraphicsPipelines的多次调用，如果缓存被存储到文件中，甚至跨越程序执行。
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);   
}

VkShaderModule DynastyEngine::createShaderModule(const std::vector<char>& code) {
    // 该函数将接收一个带有字节码的缓冲区作为参数，并从中创建一个VkShaderModule。
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    // 只需要指定一个带有字节码的缓冲区的指针和它的长度
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}


// 创建一个createUniformBuffers在之后调用 createIndexBuffer并分配缓冲区的新函数
void DynastyEngine::createUniformBuffer() {
    std::cout << "createUniformBuffer" << std::endl;
    // static auto startTime = std::chrono::high_resolution_clock::now();
    // auto currentTime = std::chrono::high_resolution_clock::now();
    // UniformBufferObject ubo{};
    // float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    // ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
    // ubo.proj[1][1] *= -1;

    // VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    // std::cout << bufferSize << std::endl;

    // VkBuffer stagingBuffer;
    // VkDeviceMemory stagingBufferMemory;

    // createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
 
    // void* data;
    // vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    // memcpy(data, &ubo, (size_t) bufferSize);
    // vkUnmapMemory(device, stagingBufferMemory);

    // createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
 
    // copyBuffer(stagingBuffer, uniformBuffer, bufferSize);
 
    // vkDestroyBuffer(device, stagingBuffer, nullptr);
    // vkFreeMemory(device, stagingBufferMemory, nullptr);

    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
    vkMapMemory(device, uniformBufferMemory, 0, bufferSize, 0, &uniformBufferMapped);
}

// 使用主机可见缓冲区作为临时缓冲区，并使用设备本地缓冲区作为实际顶点缓冲区
void DynastyEngine::createVertexBuffer() {
    std::cout << "createVertexBuffer" << std::endl;
    // 使用新的stagingBuffer、stagingBufferMemory来映射和复制顶点数据
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    std::cout << "bufferSize:" << bufferSize << std::endl;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    void* data;
    // 允许我们访问由偏移量和大小定义的指定内存资源区域。这里的偏移量和大小分别是0和 bufferInfo.size。
    // 倒数第二个参数可用于指定标志，但当前 API 中尚无可用标志。它必须设置为值0。
    // 最后一个参数指定指向映射内存的指针的输出。
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    // 简单地memcpy将顶点数据存储到映射的内存中，然后使用vkUnmapMemor
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // VK_BUFFER_USAGE_TRANSFER_SRC_BIT：缓冲区可以用作内存传输操作中的源。
    // VK_BUFFER_USAGE_TRANSFER_DST_BIT：缓冲区可用作内存传输操作中的目标。
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void DynastyEngine::createIndexBuffer() {
    std::cout << "createVertexBuffer" << std::endl;
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();//索引数bufferSize乘以索引类型的大小
 
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
 
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
 
    copyBuffer(stagingBuffer, indexBuffer, bufferSize);
 
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void DynastyEngine::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        // 缓冲区已创建，但实际上尚未分配任何内存。
        // 为缓冲区分配内存的第一步是使用适当命名的vkGetBufferMemoryRequirements 函数查询其内存需求。
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties); // 这个 properties 需要传过来

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        // vkBindBufferMemory 将此内存与缓冲区相关联
        vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

uint32_t DynastyEngine::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        // 结构有两个数组memoryTypes 和memoryHeaps。
        // 内存堆是不同的内存资源，例如专用 VRAM 和 RAM 中用于 VRAM 耗尽时的交换空间。这些堆中存在不同类型的内存。
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
}

void DynastyEngine::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    // 内存传输操作是使用命令缓冲区执行的，就像绘图命令一样。因此我们必须首先分配一个临时命令缓冲区。
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool; // 短期缓冲区创建一个单独的命令池
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    // 立即开始记录命令缓冲区
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // 只打算使用命令缓冲区一次，然后等待从函数返回，直到复制操作完成执行
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    // 使用命令传输缓冲区的内容vkCmdCopyBuffer。它以源缓冲区和目标缓冲区作为参数，以及要复制的区域数组。
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    // 命令缓冲区仅包含复制命令，因此我们可以立即停止记录。现在执行命令缓冲区以完成传输：
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    //与绘制命令不同，这次我们不需要等待任何事件。我们只想立即在缓冲区上执行传输
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void DynastyEngine::createFramebuffers() {
    // 调整容器的大小以容纳所有的帧缓冲区
    swapChainFramebuffers.resize(swapChainImageViews.size());

    // 遍历图像视图并从中创建帧缓冲区
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        // 首先需要指定renderPass帧缓冲区需要与哪个兼容
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        // 应绑定到渲染通道数组中相应附件描述的attachmentCount对象
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void DynastyEngine::createCommandPool() {
    // 命令池创建只需要两个参数
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // 每帧记录一个命令缓冲区，因此我们希望能够重置并重新记录它
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT：提示命令缓冲区经常用新命令重新记录（可能会改变内存分配行为）
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: 允许单独重新记录命令缓冲区，如果没有这个标志，它们都必须一起重置
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void DynastyEngine::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    // 该level参数指定分配的命令缓冲区是主命令缓冲区还是辅助命令缓冲区。
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void DynastyEngine::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    // 我们为每个交换链图像创建了一个帧缓冲区，它被指定为颜色附件。
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    // 定义渲染区域的大小。渲染区域定义着色器加载和存储将发生的位置
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    // 透明颜色定义为简单的黑色，不透明度为 100%。
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // 渲染过程现在可以开始了。所有记录命令的函数都可以通过它们的vkCmd前缀来识别。
    // 每个命令的第一个参数始终是用于记录命令的命令缓冲区。第二个参数指定我们刚刚提供的渲染过程的细节。最后一个参数控制如何提供渲染过程中的绘图命令。它可以具有以下两个值之一：
    // VK_SUBPASS_CONTENTS_INLINE：渲染过程命令将嵌入主命令缓冲区本身，不会执行辅助命令缓冲区。
    // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS：渲染过程命令将从辅助命令缓冲区执行。
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    // 我们现在可以绑定图形管道：
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // 视口基本上描述了输出将被渲染到的framebuffer的区域。这几乎总是（0，0）到（width, height）。
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    // 交换链及其图像的大小可能与窗口的WIDTH和HEIGHT不同。
    // 交换链的图像以后将被用作帧缓冲器，所以我们应该坚持它们的尺寸。
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    // minDepth和maxDepth值指定了用于帧缓冲区的深度值范围。这些值必须在[0.0f, 1.0f]范围内
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    // 只想在整个帧缓冲区内作画，所以我们将指定一个完全覆盖它的剪切矩形
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    // 实际将每个帧的正确描述符集绑定到着色器中的描述符vkCmdBindDescriptorSets。这需要在调用之前完成vkCmdDrawIndexed
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[0], 0, nullptr);
    
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    // vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    // 我们已经完成了命令缓冲区的记录：
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void DynastyEngine::createSyncObjects() {
    // 创建信号量
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // 创建围栏需要填写VkFenceCreateInfo
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // 在信号状态下创建围栏，以便第一次调用 vkWaitForFences()立即返回，因为围栏已经发出信号。
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

// void DynastyEngine::updateUniformBuffer(uint32_t currentImage) {
void DynastyEngine::updateUniformBuffer() {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
}

void DynastyEngine::drawFrame() {
    // 等待所有的栅栏，但在单个栅栏的情况下，这无关紧要
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    // 希望从中获取图像的逻辑设备和交换链
    // 指定图像可用的超时时间（以纳秒为单位）：使用 64 位无符号整数的最大值意味着我们可以有效地禁用超时。
    // 开始绘制的时间点，最后一个参数指定一个变量来输出已变为可用的交换链图像的索引
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    updateUniformBuffer();
    vkResetFences(device, 1, &inFlightFence);

    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // 指定在执行开始之前要等待的信号量以及要等待的管道阶段
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    // 指定了写入颜色附件的图形管道阶段
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    // 指定实际提交执行的命令缓冲区
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // 指定在命令缓冲区执行完毕后发送信号的信号量signalSemaphoreCount
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // 使用 vkQueueSubmit 将命令缓冲区提交到图形队列 
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // 绘制框架的最后一步是将结果提交回交换链，使其最终显示在屏幕上。
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // 发生之前要等待的信号量
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    // 获取将发出信号的信号量并等待它们
    // 呈现图像的交换链以及每个交换链的图像索引
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    // 向交换链提交呈现图像的请求
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(presentQueue, &presentInfo);
}


void DynastyEngine::cleanup() {
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroyFence(device, inFlightFence, nullptr);

    vkDestroyCommandPool(device, commandPool, nullptr);

    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroyDevice(device, nullptr);

    

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}


void DynastyEngine::run() {
    initEngine();
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}