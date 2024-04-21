
// #ifndef GLFW_INCLUDE_VULKAN
// #define GLFW_INCLUDE_VULKAN 1
// #endif
// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_ENABLE_EXPERIMENTAL

// #include <GLFW/glfw3.h>
// #include <vulkan/vulkan.h>
// #include <imgui.h>
// #include <imgui_internal.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

// #include <stdlib.h>
// #include <iostream>
// #include <stdexcept>
// #include <algorithm>
// #include <vector>
// #include <fstream>
// #include <chrono>
// #include <cstdint>
// #include <cstdlib>
// #include <cstring>
// #include <array>
// #include <limits>
// #include <optional>
// #include <set>
// #include <unordered_map>

// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtx/hash.hpp>

// const uint32_t WIDTH = 800;
// const uint32_t HEIGHT = 600;

// const int MAX_FRAMES_IN_FLIGHT = 2;

// const std::vector<const char*> validationLayers = {
//     "VK_LAYER_KHRONOS_validation"
// };

// const std::vector<const char*> deviceExtensions = {
//     VK_KHR_SWAPCHAIN_EXTENSION_NAME
// };

// #ifdef NDEBUG
// const bool enableValidationLayers = false;
// #else
// const bool enableValidationLayers = true;
// #endif

// struct QueueFamilyIndices {
//     std::optional<uint32_t> graphicsFamily;
//     std::optional<uint32_t> presentFamily;

//     bool isComplete() {
//         return graphicsFamily.has_value();
//     }
// };

// struct SwapChainSupportDetails {
//     VkSurfaceCapabilitiesKHR capabilities;
//     std::vector<VkSurfaceFormatKHR> formats;
//     std::vector<VkPresentModeKHR> presentModes;
// };

// struct Vertex {
//     glm::vec3 pos;
//     glm::vec3 color;
//     glm::vec2 texCoord;

//     static VkVertexInputBindingDescription getBindingDescription() {
//         VkVertexInputBindingDescription bindingDescription{};
//         bindingDescription.binding = 0;
//         bindingDescription.stride = sizeof(Vertex);
//         bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

//         return bindingDescription;
//     }

//     static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
//         std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

//         attributeDescriptions[0].binding = 0;
//         attributeDescriptions[0].location = 0;
//         attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//         attributeDescriptions[0].offset = offsetof(Vertex, pos);

//         attributeDescriptions[1].binding = 0;
//         attributeDescriptions[1].location = 1;
//         attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//         attributeDescriptions[1].offset = offsetof(Vertex, color);

//         attributeDescriptions[2].binding = 0;
//         attributeDescriptions[2].location = 2;
//         attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
//         attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

//         return attributeDescriptions;
//     }

//     bool operator==(const Vertex& other) const {
//         return pos == other.pos && color == other.color && texCoord == other.texCoord;
//     }
// };

// namespace std {
//     template<> struct hash<Vertex> {
//         size_t operator()(Vertex const& vertex) const {
//             return ((hash<glm::vec3>()(vertex.pos) ^
//              (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
//              (hash<glm::vec2>()(vertex.texCoord) << 1);
//         }
//     };
// }

// struct UniformBufferObject {
//     alignas(16) glm::mat4 model;
//     alignas(16) glm::mat4 view;
//     alignas(16) glm::mat4 proj;
// };

// class DynastyEngine {
// private:
//     /* data */
//     GLFWwindow* window;
//     VkInstance instance;
//     VkDebugUtilsMessengerEXT debugMessenger;
//     VkSurfaceKHR surface;

//     VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
//     VkDevice device;

//     VkQueue graphicsQueue;
//     VkQueue presentQueue;
    
//     VkSwapchainKHR swapChain;
//     std::vector<VkImage> swapChainImages;
//     VkFormat swapChainImageFormat;
//     VkExtent2D swapChainExtent;
//     std::vector<VkImageView> swapChainImageViews;
//     std::vector<VkFramebuffer> swapChainFramebuffers;

//     VkRenderPass renderPass;
//     VkDescriptorSetLayout descriptorSetLayout;
//     VkPipelineLayout pipelineLayout;
//     VkPipeline graphicsPipeline;

//     VkCommandPool commandPool;
    
//     VkImage depthImage;
//     VkDeviceMemory depthImageMemory;
//     VkImageView depthImageView;

//     VkImage textureImage;
//     VkDeviceMemory textureImageMemory;
//     VkImageView textureImageView;
//     VkSampler textureSampler;

//     std::vector<Vertex> vertices;
//     std::vector<uint32_t> indices;
//     VkBuffer vertexBuffer;
//     VkDeviceMemory vertexBufferMemory;
//     VkBuffer indexBuffer;
//     VkDeviceMemory indexBufferMemory;

//     std::vector<VkBuffer> uniformBuffers;
//     std::vector<VkDeviceMemory> uniformBuffersMemory;
//     std::vector<void*> uniformBuffersMapped;

//     VkDescriptorPool descriptorPool;
//     std::vector<VkDescriptorSet> descriptorSets;

//     std::vector<VkCommandBuffer> commandBuffers;

//     std::vector<VkSemaphore> imageAvailableSemaphores;
//     std::vector<VkSemaphore> renderFinishedSemaphores;
//     std::vector<VkFence> inFlightFences;
//     uint32_t currentFrame = 0;

//     bool framebufferResized = false;

//     void initWindow(); 
//     void mainLoop(); 
//     void initVulkan();
//     void recreateSwapChain();
//     void createInstance(); 
//     void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo); 
//     static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
//     void setupDebugMessenger(); 
//     std::vector<const char*> getRequiredExtensions(); 
//     bool checkValidationLayerSupport(); 
//     static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
//     void createSurface();
//     void pickPhysicalDevice();
//     bool isDeviceSuitable(VkPhysicalDevice device);
//     QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
//     VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
//     VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
//     VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
//     SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
//     void createLogicalDevice();
//     void createSwapChain();
//     void createImageViews();
//     VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
//     void createRenderPass();
//     void createDescriptorSetLayout();
//     void createGraphicsPipeline();   
//     VkShaderModule createShaderModule(const std::vector<char>& code); 
//     void createDepthResources();
//     VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
//     VkFormat findDepthFormat();
//     bool hasStencilComponent(VkFormat format);
//     void createCommandPool();
//     // void createDepthResources();
//     void createTextureImage();
//     void createTextureImageView();
//     void createTextureSampler();
//     void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
//     void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
//     void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
//     void loadModel();
//     void createVertexBuffer();
//     void createIndexBuffer();
//     void createUniformBuffers();
//     void createDescriptorPool();
//     void createDescriptorSets();
//     void createCommandBuffers();
//     void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
//     void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
//     VkCommandBuffer beginSingleTimeCommands();
//     void endSingleTimeCommands(VkCommandBuffer commandBuffer);
//     uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
//     void updateUniformBuffer(uint32_t currentImage);
//     void createFramebuffers();
//     void createCommandBuffer();
//     void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
//     void createSyncObjects();
//     void drawFrame();
//     void cleanupSwapChain();
//     void cleanup();

//     static std::vector<char> readFile(const std::string& filename) {
//         std::cerr << "filename: " << filename << std::endl;

//         std::ifstream file(filename, std::ios::ate | std::ios::binary);

//         if (!file.is_open()) {
//             throw std::runtime_error("failed to open file!");
//         }

//         size_t fileSize = (size_t) file.tellg();
//         std::vector<char> buffer(fileSize);

//         file.seekg(0);
//         file.read(buffer.data(), fileSize);

//         file.close();
//         return buffer;
//     }

// public:
//     void run();
// };