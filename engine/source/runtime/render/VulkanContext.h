#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <array>
#include <unordered_map>
#include <algorithm>

#pragma clang diagnostic ignored "-Wc++98-compat-extra-semi"
#define VMA_VULKAN_VERSION 1002000  // Vulkan 1.2
#include <vk_mem_alloc.h>

#include "runtime/code/base/Timer.h"
#include "runtime/code/base/macro.h"


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


namespace DynastyEngine
{
    const std::vector<const char*> validationLayers = 
    {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    struct QueueFamilyIndices 
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() 
        {
            try {
                return graphicsFamily.has_value();
            } catch (const std::bad_optional_access& e) {
                std::cout << "==> error: " << e.what() << std::endl;
            } catch (std::exception& e) {
                std::cout << "==> exception: " << e.what() << std::endl;
            }
        }
    };

    struct SwapChainSupportDetails 
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct AllocatedBuffer 
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VmaAllocationInfo allocInfo{};

        void destroy(VmaAllocator allocator) 
        {
            vmaDestroyBuffer(allocator, buffer, allocation);

            buffer = VK_NULL_HANDLE;
            allocation = VK_NULL_HANDLE;
        }
    };

    struct AllocatedImage 
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize allocationSize = 0;

        void destroy(VkDevice device) 
        {
            vkDestroyImage(device, image, nullptr);
            vkFreeMemory(device, memory, nullptr);

            image = VK_NULL_HANDLE;
            memory = VK_NULL_HANDLE;
        }
    };

    struct UniformBuffer 
    {
        AllocatedBuffer buffer{};
        void *mapPtr = nullptr;
        bool inUse = false;
    };

    struct DescriptorSet 
    {
        VkDescriptorSet set = VK_NULL_HANDLE;
        bool inUse = false;
    };

    struct CommandBuffer 
    {
        VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
        VkSemaphore semaphore = VK_NULL_HANDLE;
        VkFence fence = VK_NULL_HANDLE;
        std::vector<UniformBuffer *> uniformBuffers;
        std::vector<DescriptorSet *> descriptorSets;
        bool inUse = false;
    };


    class VulkanContext 
    {
    public:
        bool create(GLFWwindow* window, bool debugOutput = false);
        void destroy();

        inline const VkInstance &instance() const 
        {
            return instance_;
        }

        inline const VkPhysicalDevice &physicalDevice() const 
        {
            return physicalDevice_;
        }

        inline const VkDevice &device() const 
        {
            return device_;
        }

        inline VkQueue &graphicsQueue() 
        {
            return graphicsQueue_;
        }

        inline VkCommandPool &commandPool() 
        {
            return commandPool_;
        }

        inline VkPhysicalDeviceProperties &physicalDeviceProperties() 
        {
            return deviceProperties_;
        }

        inline VmaAllocator &allocator() 
        {
            return allocator_;
        }
        
        inline std::vector<VkImage> swapChainImages() 
        {
            return swapChainImages_;
        };
        
        inline std::vector<VkImageView> swapChainImageViews() 
        {
            return swapChainImageViews_;
        };

        inline uint32_t imageIndex() 
        {
            return imageIndex_;
        }

        inline VkExtent2D swapChainExtent() 
        {
            return swapChainExtent_;
        }

        inline VkFormat swapChainImageFormat() 
        {
            return swapChainImageFormat_;
        }

        inline VkSemaphore imageAvailableSemaphore() 
        {
            return imageAvailableSemaphore_;
        }

        inline VkSemaphore renderFinishedSemaphore() 
        {
            return renderFinishedSemaphore_;
        }

        inline VkFence inFlightFence() 
        {
            return inFlightFence_;
        }

        bool createInstance(); 
        bool setupDebugMessenger();
        bool createSurface();
        bool pickPhysicalDevice();
        bool createLogicalDevice();
        bool createCommandPool();
        bool createSwapchain();
        bool createSwapchainImageViews();
        bool createSyncObjects();

        static bool checkValidationLayerSupport();
        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
        bool isDeviceSuitable(VkPhysicalDevice device); 
        
        std::vector<const char*> getRequiredExtensions();
        bool linearBlitAvailable(VkFormat format);
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                            VkDebugUtilsMessageTypeFlagsEXT messageType, 
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();


        void createGPUBuffer(AllocatedBuffer &buffer, VkDeviceSize size, VkBufferUsageFlags usage);
        void createUniformBuffer(AllocatedBuffer &buffer, VkDeviceSize size);
        void createStagingBuffer(AllocatedBuffer &buffer, VkDeviceSize size);
        bool createImageMemory(AllocatedImage &image, uint32_t properties, const void *pNext = nullptr);
        UniformBuffer *getNewUniformBuffer(VkDeviceSize size);

        CommandBuffer* beginCommands();
        void endCommands(CommandBuffer *commandBuffer, const std::vector<VkSemaphore> &waitSemaphores = {},  const std::vector<VkSemaphore> &signalSemaphores = {});
        void waitCommands(CommandBuffer *commandBuffer);

        void allocateCommandBuffer(VkCommandBuffer &cmdBuffer);
        CommandBuffer* getNewCommandBuffer();
        uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);
        void purgeCommandBuffers();

        void beforeDrawFrame();
        void afterDrawFrame(VkCommandBuffer &drawCmd);
        
        void recreateSwapChain();
        void cleanupSwapChain();

    private:
        GLFWwindow* window_;
        bool debugOutput_ = false;
        VkInstance instance_;
        VkDebugUtilsMessengerEXT debugMessenger_;
        VkSurfaceKHR surface_;

        VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
        VkDevice device_;

        QueueFamilyIndices queueIndices_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;

        VkSwapchainKHR swapChain_;
        // 这些图像是由交换链的实现创建的，一旦交换链被销毁，它们将被自动清理
        std::vector<VkImage> swapChainImages_;
        VkFormat swapChainImageFormat_;
        VkExtent2D swapChainExtent_;
        // 图像视图实际上是对图像的一种观察。它描述了如何访问图像以及访问图像的哪一部分
        // 编写一个createImageViews函数，为交换链中的每个图像创建一个基本的图像视图
        // 这样我们就可以在以后将它们作为颜色目标。
        std::vector<VkImageView> swapChainImageViews_;
        std::vector<VkFramebuffer> swapChainFramebuffers_;
        uint32_t imageIndex_ = 0;

        VkSemaphore imageAvailableSemaphore_;
        VkSemaphore renderFinishedSemaphore_;
        VkFence inFlightFence_;
        
        // 在创建命令缓冲区之前，我们必须创建一个命令池。命令池管理用于存储缓冲区的内存，命令缓冲区是从它们中分配的。添加一个新的类成员来存储一个VkCommandPool
        VkCommandPool commandPool_;

        // command buffer pool
        std::vector<CommandBuffer> commandBuffers_;
        // uniform buffer pool
        std::unordered_map<VkDeviceSize, std::vector<UniformBuffer>> uniformBufferPool_;

        size_t maxCommandBufferPoolSize_ = 0;
        size_t maxUniformBufferPoolSize_ = 0;

        std::unordered_map<std::string, VkExtensionProperties> instanceExtensions_;
        std::unordered_map<std::string, VkExtensionProperties> deviceExtensions_;

        VkPhysicalDeviceProperties deviceProperties_{};
        VkPhysicalDeviceMemoryProperties deviceMemoryProperties_{};

        VmaAllocator allocator_ = VK_NULL_HANDLE;
    };
}
