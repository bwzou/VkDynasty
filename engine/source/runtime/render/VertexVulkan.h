#pragma once

#include "runtime/code/base/UUID.h"
#include "runtime/code/base/Timer.h"
#include "runtime/function/Vertex.h"
#include "VulkanUtils.h"

namespace DynastyEngine
{
    class VertexArrayObjectVulkan : public VertexArrayObject 
    {
    public:
        VertexArrayObjectVulkan(VulkanContext  &ctx, const VertexArray &vertexArr) : vkCtx_(ctx) 
        {
            device_ = ctx.device();
            if (!vertexArr.vertexesBuffer || !vertexArr.indexBuffer) 
            {
                return ;
            }
            indicesCnt_ = vertexArr.indexBufferLength / sizeof(int32_t);

            bindDescription_.binding = 0;
            bindDescription_.stride = vertexArr.vertexSize;
            bindDescription_.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            size_t attrCnt = vertexArr.vertexesDesc.size();
            attributeDescriptions_.resize(attrCnt);
            for (size_t i = 0;  i < attrCnt; i++) 
            {
                auto &attrDesc = vertexArr.vertexesDesc[i];
                attributeDescriptions_[i].binding = 0;
                attributeDescriptions_[i].location  = i;
                attributeDescriptions_[i].format = vertexAttributeFormat(attrDesc.size);
                attributeDescriptions_[i].offset = attrDesc.offset;
            }

            vertexInputInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo_.vertexBindingDescriptionCount = 1;
            vertexInputInfo_.pVertexBindingDescriptions = &bindDescription_;
            vertexInputInfo_.vertexAttributeDescriptionCount = attributeDescriptions_.size();
            vertexInputInfo_.pVertexAttributeDescriptions = attributeDescriptions_.data();

            // create buffers
            vkCtx_.createGPUBuffer(vertexBuffer_, vertexArr.vertexesBufferLength, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            vkCtx_.createGPUBuffer(indexBuffer_, vertexArr.indexBufferLength, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
            
            // create
            vkCtx_.createStagingBuffer(vertexStagingBuffer_, vertexArr.vertexesBufferLength);
            vkCtx_.createStagingBuffer(indexStagingBuffer_, vertexArr.indexBufferLength);
            
            // upload data
            uploadBufferData(vertexBuffer_, vertexStagingBuffer_, vertexArr.vertexesBuffer, vertexArr.vertexesBufferLength, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
            uploadBufferData(indexBuffer_, indexStagingBuffer_, vertexArr.indexBuffer, vertexArr.indexBufferLength, VK_ACCESS_INDEX_READ_BIT);
        }
        ~VertexArrayObjectVulkan() {
            vertexBuffer_.destroy(vkCtx_.allocator());
            indexBuffer_.destroy(vkCtx_.allocator());
            vertexStagingBuffer_.destroy(vkCtx_.allocator());
            indexStagingBuffer_.destroy(vkCtx_.allocator());
        }

        void updateVertexData(void *data, size_t length) override 
        {
            uploadBufferData(vertexBuffer_, vertexStagingBuffer_, data, length, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
        }

        int getId() const override 
        {
            return uuid_.get();
        }

        // only Float element
        static VkFormat vertexAttributeFormat(size_t size) 
        {
            switch (size) {
            case 1: return VK_FORMAT_R32_SFLOAT;
            case 2: return VK_FORMAT_R32G32_SFLOAT;
            case 3: return VK_FORMAT_R32G32B32_SFLOAT;
            case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
            case 5: return VK_FORMAT_R32G32B32A32_SINT;
            default:
                break;
            }
            return VK_FORMAT_UNDEFINED;
        }

        inline VkPipelineVertexInputStateCreateInfo &getVertexInputInfo() 
        {
            return vertexInputInfo_;
        }

        inline uint32_t getIndicesCnt() const 
        {
            return indicesCnt_;
        }

        inline VkBuffer &getVertexBuffer() 
        {
            return vertexBuffer_.buffer;
        }

        inline VkBuffer &getIndexBuffer() 
        {
            return indexBuffer_.buffer;
        }


    private:
        void uploadBufferData(AllocatedBuffer &buffer, AllocatedBuffer &stagingBuffer, void *bufferData, 
                            VkDeviceSize bufferSize,  VkAccessFlags dstAccessMask) 
        {
            memcpy(stagingBuffer.allocInfo.pMappedData, bufferData, (size_t) bufferSize);
            
            auto *commandBuffer = vkCtx_.beginCommands();

            VkBufferCopy copyRegion{};
            copyRegion.size = bufferSize;
            // 使用命令传输缓冲区的内容vkCmdCopyBuffer。它以源缓冲区和目标缓冲区作为参数，以及要复制的区域数组。
            vkCmdCopyBuffer(commandBuffer->cmdBuffer, stagingBuffer.buffer, buffer.buffer, 1, &copyRegion);

            // use barrier to ensure that data is uploaded to the GPU before it is accessed
            VkBufferMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | dstAccessMask;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.buffer = buffer.buffer;
            barrier.offset = 0;
            barrier.size = VK_WHOLE_SIZE;
            vkCmdPipelineBarrier(commandBuffer->cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);

            vkCtx_.endCommands(commandBuffer);
        }


    private:
        UUID<VertexArrayObjectVulkan> uuid_;
        VulkanContext& vkCtx_;
        VkDevice device_;

        uint32_t indicesCnt_ = 0;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo_{};
        VkVertexInputBindingDescription bindDescription_{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions_;

        AllocatedBuffer vertexBuffer_{};
        AllocatedBuffer indexBuffer_{};

        AllocatedBuffer vertexStagingBuffer_{};
        AllocatedBuffer indexStagingBuffer_{};
    };
}