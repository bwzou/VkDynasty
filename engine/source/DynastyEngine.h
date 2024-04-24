
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "./code/base/GLMInc.h"

#include "json11.hpp"

#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <fstream>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <array>
#include <limits>
#include <optional>
#include <set>
#include <unordered_map>

#include "./function/Model.h"
#include "./function/Material.h"
#include "./function/Config.h"
#include "./function/ModelLoader.h"
#include "./code/util/FileUtils.h"


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};





class DynastyEngine {
private:
    /* data */
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    
    VkSwapchainKHR swapChain;
    // 这些图像是由交换链的实现创建的，一旦交换链被销毁，它们将被自动清理
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    // 图像视图实际上是对图像的一种观察。它描述了如何访问图像以及访问图像的哪一部分
    // 编写一个createImageViews函数，为交换链中的每个图像创建一个基本的图像视图
    // 这样我们就可以在以后将它们作为颜色目标。
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    // 所有描述符绑定都组合到一个 VkDescriptorSetLayout对象中。
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    // 在创建命令缓冲区之前，我们必须创建一个命令池。命令池管理用于存储缓冲区的内存，命令缓冲区是从它们中分配的。添加一个新的类成员来存储一个VkCommandPool
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // std::vector<Vertex> vertices;
    //把索引的类型从uint16_t改为uint32_t，因为顶点会比65535多很多。
    // std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    void* uniformBufferMapped;

    VkBuffer materialBuffer;
    VkDeviceMemory materialBufferMemory;
    void* materialBufferMapped;

    // std::vector<VkBuffer> uniformBuffers;
    // std::vector<VkDeviceMemory> uniformBuffersMemory;
    // std::vector<void*> uniformBuffersMapped;


    // 我们需要一个信号量来表示图像已从交换链获取并准备渲染，另一个信号量表示渲染已完成并且可以进行展示，以及一个栅栏以确保一次只渲染一帧.(需要主机等待。这样我们就不会一次绘制超过一帧)
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    // 现在我们的渲染循环有一个明显的缺陷。我们需要等待前一帧完成，然后才能开始渲染下一帧，这会导致主机不必要的空闲。
    const int MAX_FRAMES_IN_FLIGHT = 2;

    uint32_t imageIndex;

    std::vector<Vertex> vertices = {
        // {{1.0f, 0.0f, 0.0f}, {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        // {{0.0f, 1.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        // {{0.0f, 0.0f, 1.0f}, {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
        { {1.0f, 0.0f, 0.0f}, {-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        { {0.0f, 1.0f, 0.0f}, {0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        { {0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        { {1.0f, 1.0f, 1.0f}, {-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
    };

    std::vector<int32_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    void initWindow(); 
    void mainLoop(); 
    void initVulkan();
    void createInstance(); 
    
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo); 
    void setupDebugMessenger(); 
    std::vector<const char*> getRequiredExtensions(); 
    bool checkValidationLayerSupport(); 
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    
    void createSurface();
    void pickPhysicalDevice();
    
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
    void createGraphicsPipeline();   
    VkShaderModule createShaderModule(const std::vector<char>& code); 
    void createCommandPool();
    void createVertexBuffer();
    void createIndexBuffer();
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkAccessFlags dstAccessMask);
    void createUniformBuffer();
    void createMaterialBuffer();
    // void updateUniformBuffer(uint32_t currentImage);
     void updateUniformBuffer();
    void createFramebuffers();
    void createCommandBuffer();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();
    void drawFrame();
    void cleanup();

    static std::vector<char> readFile(const std::string& filename) {
        std::cerr << "filename: " << filename << std::endl;

        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

public:
    int count = 0;
    std::shared_ptr<Config> config;
    std::shared_ptr<ModelLoader> modelLoader;
    std::unordered_map<std::string, std::string> modelPaths;
    std::unordered_map<std::string, std::string> skyboxPaths;
    std::vector<const char *> modelNames;
    std::vector<const char *> skyboxNames;

public:
    void run();

    void initEngine() {
        config = std::make_shared<Config>();
        modelLoader = std::make_shared<ModelLoader>(*config);
        loadConfig();
    }

    bool loadConfig() {
        auto configPath = ASSETS_DIR + "assets.json";
        auto configStr = FileUtils::readText(configPath);
        if (configStr.empty()) {
            std::cout << "load models failed: error read config file" << std::endl;
            std::cout << configPath << std::endl;
            return false;
        }

        std::string err;
        const auto json = json11::Json::parse(configStr, err);
        for (auto &kv : json["model"].object_items()) {
            modelPaths[kv.first] = ASSETS_DIR + kv.second["path"].string_value();
        }
        for (auto &kv : json["skybox"].object_items()) {
            skyboxPaths[kv.first] = ASSETS_DIR + kv.second["path"].string_value();
        }

        if (modelPaths.empty()) {
            std::cout << "load models failed: %s" << err.c_str() << std::endl;
            return false;
        }

        loadModel(modelPaths.begin()->second);
    }

    void loadModel(const std::string &filepath) {
        std::cout << "loading models..." << std::endl;
        modelLoader->loadModel(filepath);
    }

    void drawModel() {
        auto scene_ = modelLoader->scene_;
        ModelNode &modelNode = scene_.model->rootNode;
        // draw model nodes opaque
        count = 0;
        drawModelNodes(modelNode, scene_.model->centeredTransform, Alpha_Opaque, false);
        std::cout << "Alpha_Opaque model nodes: " << count << std::endl;

        // draw model nodes blend
        count = 0;
        drawModelNodes(modelNode, scene_.model->centeredTransform, Alpha_Blend, false);
        std::cout << "Alpha_Blend model nodes: " << count << std::endl;
    }

    void drawModelNodes(ModelNode &node, glm::mat4 &transform, AlphaMode mode, float specular) {
        glm::mat4 modelMatrix = transform * node.transform;

        // update model uniform
        // updateUniformModel(modelMatrix, camera_->viewMatrix());

        // draw nodes
        // std::cout<< "node.meshes" << "    " << node.meshes.size() << std::endl;
        for (auto &mesh : node.meshes) {
            if (mesh.material->alphaMode != mode) {
                continue;
            }
            // frustum cull
            // if (!checkMeshFrustumCull(mesh, modelMatrix)) {
            //     return;
            // }
            count ++;
            drawModelMesh(mesh, specular);
        }

        // draw child
        // std::cout<< "node.children" << "    " << node.children.size() << std::endl;
        for (auto &childNode : node.children) {
            drawModelNodes(childNode, modelMatrix, mode, specular);
        }
    }

    void drawModelMesh(ModelMesh &mesh, float specular) {
        // update material
        updateUniformMaterial(*mesh.material, specular);

        // update IBL textures
        // if (mesh.material->shadingModel == Shading_PBR) {
        //     updateIBLTextures(mesh.material->materialObj.get());
        // }

        // // update shadow textures
        // if (config_.shadowMap) {
        //     updateShadowTextures(mesh.material->materialObj.get(), shadowPass);
        // }

        // draw mesh
        pipelineDraw(mesh);
    }

    void pipelineDraw(ModelBase &model) {
        // auto &materialObj = model.material->materialObj;

        vertices = model.vertexes;
        indices = model.indices;
        // std::cout<< vertices.size() << "indices:  " << indices.size() << std::endl;
        createVertexBuffer();
        createIndexBuffer();
        
        vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffer, imageIndex);

        // renderer_->setVertexArrayObject(model.vao);
        // renderer_->setShaderProgram(materialObj->shaderProgram);
        // renderer_->setShaderResources(materialObj->shaderResources);
        // renderer_->setPipelineStates(materialObj->pipelineStates);
        // renderer_->draw();      
    }

    // void updateUniformModel() {
        
    // }

    void updateUniformMaterial(Material &material, float specular) {
        static UniformsMaterial uniformsMaterial{};

        // uniformsMaterial.u_enableLight = config_.showLight ? 1u : 0u;
        // uniformsMaterial.u_enableIBL = iBLEnabled() ? 1u : 0u;
        // uniformsMaterial.u_enableShadow = config_.shadowMap ? 1u : 0u;

        uniformsMaterial.u_enableLight = 0u;
        uniformsMaterial.u_enableIBL = 0u;
        uniformsMaterial.u_enableShadow = 0u;

        uniformsMaterial.u_pointSize = material.pointSize;
        uniformsMaterial.u_kSpecular = specular;
        uniformsMaterial.u_baseColor = material.baseColor;
        
        // std::cout << "update uniform material" << std::endl;
        
        memcpy(materialBufferMapped, &uniformsMaterial, sizeof(uniformsMaterial));

        // uniformBlockMaterial_->setData(&uniformsMaterial, sizeof(UniformsMaterial));
    }
};  