#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vector>


namespace DynastyEngine
{
    
    class VulkanAPI;
    class RenderResource
    {
    
    public:
        RenderResource(/* args */){}
        ~RenderResource(){}
        void clear();

        void createVertexBuffer(std::shared_ptr<VulkanAPI> mVulkanAPI);
        void createIndexBuffer(std::shared_ptr<VulkanAPI> mVulkanAPI);
        void createUniformBuffer(std::shared_ptr<VulkanAPI> mVulkanAPI);
        void updateUniformBuffer(std::shared_ptr<VulkanAPI> mVulkanAPI, uint32_t currentFrameIndex);
        void updateMeshData(std::shared_ptr<VulkanAPI>      mVulkanAPI,
                            uint32_t                        indexBufferSize,
                            void*                           indexBufferData,
                            uint32_t                        vertexBufferSize,
                            void*                           vertexBufferData);
        void updateIndexBuffer(std::shared_ptr<VulkanAPI>   mVulkanAPI,
                            uint32_t                        indexBufferSize,
                            void*                           indexBufferData);
        void updateVertexBuffer(std::shared_ptr<VulkanAPI>  mVulkanAPI,
                            uint32_t                        vertexBufferSize,
                            void*                           vertexBufferData);                            

    public:
        VkBuffer                    vertexBuffer;
        VkDeviceMemory              vertexBufferMemory;
        VkBuffer                    indexBuffer;
        VkDeviceMemory              indexBufferMemory;
        std::vector<VkBuffer>       uniformBuffers;   
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        std::vector<void*>          uniformBuffersMapped; 

    private:
        /* data */

    };

    
}
