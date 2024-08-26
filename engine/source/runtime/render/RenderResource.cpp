#include "runtime/render/RenderResource.h"
#include "runtime/platform/vulkan/VulkanAPI.h"
#include "runtime/render/RenderMesh.h"


namespace DynastyEngine 
{
    void RenderResource::clear()
    {
        
    }

    void RenderResource::createVertexBuffer(std::shared_ptr<VulkanAPI> mVulkanAPI) {
        VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
        LOG_INFO("vertexBufferSize: {} {}", vertexBufferSize, vertices.size());

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        void* data;
        // VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source in a memory transfer operation
        mVulkanAPI->createBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        mVulkanAPI->mapMemory(stagingBufferMemory, 0, vertexBufferSize, 0,  &data);
        memcpy(data, vertices.data(), (size_t) vertexBufferSize);
        mVulkanAPI->unmapMemory(stagingBufferMemory);

        LOG_INFO("createBuffer vertexBufferSize");

        // VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation.
        mVulkanAPI->createBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        LOG_INFO("createBuffer vertexBuffer");

        mVulkanAPI->copyBuffer(stagingBuffer, vertexBuffer, 0, 0, vertexBufferSize);

        mVulkanAPI->destroyBuffer(stagingBuffer);
        mVulkanAPI->freeMemory(stagingBufferMemory);
    }

    void RenderResource::createIndexBuffer(std::shared_ptr<VulkanAPI> mVulkanAPI) {
        VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
        LOG_INFO("indexBufferSize:  {}", indexBufferSize);

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        void* data;
        mVulkanAPI->createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        mVulkanAPI->mapMemory(stagingBufferMemory, 0, indexBufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t)indexBufferSize);
        
        mVulkanAPI->unmapMemory(stagingBufferMemory);

        mVulkanAPI->createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        mVulkanAPI->copyBuffer(stagingBuffer, indexBuffer, 0, 0, indexBufferSize);

        mVulkanAPI->destroyBuffer(stagingBuffer);
        mVulkanAPI->freeMemory(stagingBufferMemory);
    }

    void RenderResource::createUniformBuffer(std::shared_ptr<VulkanAPI> mVulkanAPI)
    {
        VkDeviceSize uniformBufferSize = sizeof(UniformBufferObject);
        uint32_t framesInFlight = mVulkanAPI->kMaxFramesInFlight;

        uniformBuffers.resize(framesInFlight);
        uniformBuffersMemory.resize(framesInFlight);
        uniformBuffersMapped.resize(framesInFlight);

        for (size_t i = 0; i < framesInFlight; i++) {
            mVulkanAPI->createBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

            mVulkanAPI->mapMemory(uniformBuffersMemory[i], 0, uniformBufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    void RenderResource::updateUniformBuffer(std::shared_ptr<VulkanAPI> mVulkanAPI, uint32_t currentFrameIndex) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(80.0f, 80.0f, 80.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // 第一组坐标：摄像机位置、第二组坐标：目标位置、第三组坐标：世界空间中的上向量（可以理解为摄像机顶部朝向的方向）
        LOG_INFO("width/height {} {} {}", mVulkanAPI->getSwapchainInfo().extent.width, mVulkanAPI->getSwapchainInfo().extent.height, mVulkanAPI->getSwapchainInfo().extent.width / (float) mVulkanAPI->getSwapchainInfo().extent.height);
        ubo.proj = glm::perspective(glm::radians(75.0f), mVulkanAPI->getSwapchainInfo().extent.width / (float) mVulkanAPI->getSwapchainInfo().extent.height, 0.1f, 500.0f);
        ubo.proj[1][1] *= -1;

        memcpy(uniformBuffersMapped[currentFrameIndex], &ubo, sizeof(ubo));
    }

    void RenderResource::updateMeshData(std::shared_ptr<VulkanAPI>      mVulkanAPI,
                                        uint32_t                        indexBufferSize,
                                        void*                           indexBufferData,
                                        uint32_t                        vertexBufferSize,
                                        void*                           vertexBufferData)
    {
        updateVertexBuffer(mVulkanAPI, vertexBufferSize, vertexBufferData);
        updateIndexBuffer(mVulkanAPI, indexBufferSize, indexBufferData);
    }
    
    void RenderResource::updateIndexBuffer(std::shared_ptr<VulkanAPI>   mVulkanAPI,
                                        uint32_t                        indexBufferSize,
                                        void*                           indexBufferData)
    {
        
    }
    
    void RenderResource::updateVertexBuffer(std::shared_ptr<VulkanAPI>  mVulkanAPI,
                                        uint32_t                        vertexBufferSize,
                                        void*                           vertexBufferData)
    {

    }
}