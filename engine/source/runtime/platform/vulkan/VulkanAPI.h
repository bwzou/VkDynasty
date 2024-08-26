#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "runtime/code/base/macro.h"

#pragma clang diagnostic ignored "-Wc++98-compat-extra-semi"
#define VMA_VULKAN_VERSION 1002000  // Vulkan 1.2
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include <stdlib.h>
#include <stdexcept>
#include <string>
#include <array>
#include <unordered_map>
#include <algorithm>

#include "runtime/platform/WindowSystem.h"


#define COMMAND_BUFFER_POOL_MAX_SIZE 128
#define UNIFORM_BUFFER_POOL_MAX_SIZE 128

namespace DynastyEngine 
{
    struct VulkanInitInfo
    {
        std::shared_ptr<WindowSystem> windowSystem;
    };

    struct FramebufferCreateInfo
    {
        VkStructureType          sType;
        const void*              pNext;
        VkFramebufferCreateFlags flags;
        VkRenderPass*            renderPass;
        uint32_t                 attachmentCount;
        std::vector<VkImageView> pAttachments;
        uint32_t                 width;
        uint32_t                 height;
        uint32_t                 layers;
    };

    struct QueueFamilyIndices 
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;

        bool isComplete() 
        {
            try {
                return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
            } catch (std::bad_optional_access& e) {
                std::cout << "==> error: " << e.what() << std::endl;
            } 
        }
    };

    struct SwapChainSupportDetails 
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    struct SwapChainDesc
    {
        VkExtent2D                extent;
        VkFormat                  imageFormat;
        VkViewport                viewport;
        VkRect2D                  scissor;
        std::vector<VkImageView>  imageViews;
    };

    struct DepthImageDesc
    {
        VkImage      depthImage = VK_NULL_HANDLE;
        VkImageView  depthImageView = VK_NULL_HANDLE;
        VkFormat     depthImageFormat;
    };

    class VulkanAPI
    {
    public:
        void initialize(VulkanInitInfo initInfo);

        void createInstance();
        bool initializeDebugMessenger();
        void createWindowSurface();
        void initializePhysicalDevice();
        void createLogicalDevice();
        void createSwapchain();
        void recreateSwapchain();
        void createSwapchainImageViews();
        void createFramebufferImageAndView();
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory& bufferMemory);
        bool createBufferVMA(VmaAllocator allocator, const 
                             VkBufferCreateInfo pBufferCreateInfo, 
                             const VmaAllocationCreateInfo* pAllocationCreateInfo, 
                             VkBuffer &pBuffer, 
                             VmaAllocation* pAllocation, 
                             VmaAllocationInfo* pAllocationInfo);
        bool copyBuffer(VkBuffer     srcBuffer,
                        VkBuffer     dstBuffer,
                        VkDeviceSize srcOffset,
                        VkDeviceSize dstOffset,
                        VkDeviceSize size);
        void createShaderModule(VkShaderModule  &shaderModule, const std::vector<char>& code);
        void createShaderStages(std::vector<VkShaderModule> shaders, 
                                VkPipelineShaderStageCreateInfo &vertPipelineShaderStageCreateInfo,
                                VkPipelineShaderStageCreateInfo &fragPipelineShaderStageCreateInfo);
        bool createImage(uint32_t              imageWidth,
                         uint32_t              imageHeight,
                         VkFormat              format,
                         VkImageTiling         imageTiling,
                         VkImageUsageFlags     imageUsageFlags,
                         VkMemoryPropertyFlags memoryPropertyFlags,
                         VkImage&              image,
                         VkDeviceMemory&       memory,
                         VkImageCreateFlags    imageCreateFlags,
                         uint32_t              arrayLayers,
                         uint32_t              miplevels);
        bool createImageView(VkImage&           image,
                             VkFormat           format,
                             VkImageAspectFlags imageAspectFlags,
                             VkImageViewType    viewType,
                             uint32_t           layoutCount,
                             uint32_t           miplevels,
                             VkImageView&       imageView);
        bool createFramebuffer(const FramebufferCreateInfo* pCreateInfo, VkFramebuffer &pFramebuffer);
        bool createGraphicsPipelines(const VkPipelineCache &pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo& pCreateInfos, VkPipeline &pPipelines);
        bool createComputePipelines(VkPipelineCache &pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo pCreateInfos, VkPipeline &pPipelines);
        bool createPipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, VkPipelineLayout &pPipelineLayout);
        bool createRenderPass(VkRenderPassCreateInfo renderpassCreateInfo, VkRenderPass &renderPass);
        bool createCommandPool();
        void resetCommandPool();
        void createCommandBuffers();
        bool createDescriptorPool();
        bool createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo &postProcessGlobalLayoutCreateInfo, VkDescriptorSetLayout &layout);

        bool createSyncPrimitives();
        
        void createVmaAllocator();
        bool allocateDescriptorSets(VkDescriptorSetAllocateInfo &allocateInfo, VkDescriptorSet &descriptorSet);
        bool allocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllacateInfo, VkCommandBuffer &pCommandBuffers);

        bool createFence(const VkFenceCreateInfo pCreateInfo, VkFence &pFence);
        bool waitForFences();
        bool waitForFences(uint32_t fenceCount, const VkFence* const* pFences, VkBool32 waitAll, uint64_t timeout);
        bool resetFences(uint32_t fenceCount, std::vector<VkFence> fences);

        bool createSemaphore(const VkSemaphoreCreateInfo pCreateInfo, VkSemaphore &pSemaphore);
        bool createSampler(const VkSamplerCreateInfo pCreateInfo, VkSampler &pSampler);

        bool beginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
        bool endCommandBuffer(VkCommandBuffer commandBuffer);
        void cmdCopyImageToBuffer(
            VkCommandBuffer commandBuffer,
            VkImage srcImage,
            VkImageLayout srcImageLayout,
            VkBuffer &dstBuffer,
            uint32_t regionCount,
            std::vector<VkBufferImageCopy> pRegions);
        void cmdCopyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageAspectFlagBits srcFlag, VkImage &dstImage, VkImageAspectFlagBits dstFlag, uint32_t width, uint32_t height);

        bool cmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo &pRenderPassBegin, VkSubpassContents contents);
        bool cmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
        void cmdEndRenderPass(VkCommandBuffer commandBuffer);
        void cmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);

        void cmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport pViewports);
        void cmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D pScissors);
        void cmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
        void cmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, std::vector<VkBuffer> pBuffers, std::vector<VkDeviceSize> pOffsets);
        void cmdBindDescriptorSets(
            VkCommandBuffer commandBuffer,
            VkPipelineBindPoint pipelineBindPoint,
            VkPipelineLayout layout,
            uint32_t firstSet,
            uint32_t descriptorSetCount,
            std::vector<VkDescriptorSet> pDescriptorSets,
            uint32_t dynamicOffsetCount,
            std::vector<uint32_t> pDynamicOffsets);
        void cmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
        void cmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer &srcBuffer, VkBuffer &dstBuffer, uint32_t regionCount, VkBufferCopy pRegions);
        void cmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

        void updateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies);
        void queueSubmit(VkQueue queue, uint32_t submitCount, std::vector<VkSubmitInfo> pSubmits, VkFence fence);
        void queueWaitIdle(VkQueue queue);

        void prepareContext();
        bool prepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain);
        void submitAfterPass(std::function<void()> passUpdateAfterRecreateSwapchain);

        // query
        VkCommandPool   getCommandPoor() const;
        VkCommandBuffer getCurrentCommandBuffer() const;
        VkQueue         getGraphicsQueue() const;
        VkQueue         getComputeQueue() const;
        SwapChainDesc   getSwapchainInfo();
        DepthImageDesc  getDepthImageInfo() const;
        uint8_t         getCurrentFrameIndex();
        void            setCurrentFrameIndex(uint8_t index);
        uint8_t         getMaxFramesInFlight() const;
        

        // command write
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer& commandBuffer);
        void pushEvent();
        void popEvent();

        // destory
        void clearSwapchain();
        void destroyShaderModule(VkShaderModule &shaderModule);
        void destroySemaphore(VkSemaphore semaphore);
        void destroyInstance();
        void destroyImageView(VkImageView imageView);
        void destroyImage(VkImage image);
        void destroyFramebuffer(VkFramebuffer framebuffer);
        void destroyFence(VkFence fence);
        void destroyDevice();
        void destroyCommandPool(VkCommandPool commandPool);
        void destroyBuffer(VkBuffer &buffer);
        void freeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount, VkCommandBuffer pCommandBuffers);

        void clear();

        // memory
        void freeMemory(VkDeviceMemory &memory);
        bool mapMemory(VkDeviceMemory &memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
        void unmapMemory(VkDeviceMemory &memory);
        uint32_t                findMemoryType(VkPhysicalDevice      physical_device,
                                               uint32_t              type_filter,
                                               VkMemoryPropertyFlags properties_flag);
        
        bool                     checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        void                     populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        VkResult                 createDebugUtilsMessengerEXT(VkInstance                                instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks*              pAllocator,
                                                              VkDebugUtilsMessengerEXT*                 pDebugMessenger);
        void                     destroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                                               VkDebugUtilsMessengerEXT     debugMessenger,
                                                               const VkAllocationCallbacks* pAllocator);
        QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice physicalDevice);
        bool                    checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
        bool                    isDeviceSuitable(VkPhysicalDevice device); 
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physical_device);                                                      
        
        VkFormat                findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat                findDepthFormat();
        VkSurfaceFormatKHR      chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR        chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D              chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities);

        VkSampleCountFlagBits   getMaxUsableSampleCount();

        
    public:
        static uint8_t const           kMaxFramesInFlight { 3 };
        const std::vector<char const*> mValidationLayers { "VK_LAYER_KHRONOS_validation" };
        std::vector<char const*>       mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        bool                           pEnableValidationLayers{ true };
        bool                           pEnableDebugUtilsLabel{ true };

        VkDebugUtilsMessengerEXT       mDebugMessenger = nullptr;
        GLFWwindow*                    mWindow { nullptr };
        VkInstance                     mInstance { nullptr };
        VkSurfaceKHR                   mSurface { nullptr };
        VkPhysicalDevice               mPhysicalDevice { nullptr };
        VkDevice                       mDevice { nullptr };
        
        VkQueue                        mPresentQueue { nullptr };
        VkQueue                        mGraphicsQueue{ nullptr };
        VkQueue                        mComputeQueue{ nullptr };

        VkSwapchainKHR                 mSwapchain { nullptr };
        std::vector<VkImage>           mSwapchainImages;
        std::vector<VkImageView>       mSwapchainImageviews;
        VkFormat                       mSwapchainImageFormat;
        std::vector<VkFramebuffer>     mSwapchainFramebuffers;
        VkExtent2D                     mSwapchainExtent;

        VmaAllocator                   mVmaAllocator;

        VkCommandBuffer                mCommandBuffers[kMaxFramesInFlight];
        VkCommandBuffer                mCurrentCommandBuffer;
        VkCommandPool                  mCommandPool; 
        VkDescriptorPool               mDescriptorPool;
        VkCommandPool                  mCommandPools[kMaxFramesInFlight];
        
        VkViewport                     mViewport;
        VkRect2D                       mScissor;

        VkSemaphore                    mImageAvailableForRenderSemaphores[kMaxFramesInFlight];
        VkSemaphore                    mImageFinishedForPresentationSemaphores[kMaxFramesInFlight];
        VkFence                        mIsFrameInFlightFences[kMaxFramesInFlight];

        QueueFamilyIndices             mQueueIndices;

        VkImage                        mDepthImage;
        VkDeviceMemory                 mDepthImageMemory { nullptr };
        VkFormat                       mDepthImageFormat{ VK_FORMAT_UNDEFINED };
        VkImageView                    mDepthImageView;

        uint8_t                        mCurrentFrameIndex { 0 };
        uint32_t                       mCurrentSwapchainImageIndex{ 0 };
    };
}
 