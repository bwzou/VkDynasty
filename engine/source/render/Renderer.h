#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif
#include <GLFW/glfw3.h>
#include "FrameBuffer.h"
#include "VulkanContext.h"
#include "PipelineStates.h"
#include "ShaderProgram.h"
#include "VertexVulkan.h"
#include "UniformVulkan.h"
#include "../function/Texture.h"
#include "../code/base/macro.h"


namespace DynastyEngine
{
    class Renderer 
    {
    public:
        Renderer(/* args */){}
        ~Renderer(){}

        bool create(GLFWwindow* window);
        bool destroy();
        
        // framebuffer
        std::shared_ptr<FrameBuffer> createFrameBuffer(bool offscreen);

        // texture
        std::shared_ptr<Texture> createTexture(const TextureDesc &desc);

        // vertex
        std::shared_ptr<VertexArrayObject> createVertexArrayObject(const VertexArray &vertexArray);

        // shader program
        std::shared_ptr<ShaderProgram> createShaderProgram();

        // pipeline states
        std::shared_ptr<PipelineStates> createPipelineStates(const RenderStates &renderStates); 

        // uniform
        std::shared_ptr<UniformBlock> createUniformBlock(const std::string &name, int size);
        std::shared_ptr<UniformSampler> createUniformSampler(const std::string &name, const TextureDesc &desc);

        // pipeline
        void beginRender();
        void endRender();
        
        void beginRenderPass(std::shared_ptr<FrameBuffer> &frameBuffer, const ClearStates &states);
        void setViewPort(int x, int y, int width, int height);
        void setVertexArrayObject(std::shared_ptr<VertexArrayObject> &vao);
        void setShaderProgram(std::shared_ptr<ShaderProgram> &program);
        void setShaderResources(std::shared_ptr<ShaderResources> &resources);
        void setPipelineStates(std::shared_ptr<PipelineStates> &states);
        void draw();
        void setCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
        void endRenderPass(); 
        void waitIdle();

        inline VulkanContext &getVkCtx() 
        {
            return vkCtx_;
        }
        inline FrameBuffer *getFbo() 
        {
            return fbo_;
        }
        inline PipelineStates *getPipelineStates() 
        {
            return pipelineStates_;
        }
        inline ShaderProgram *getShaderProgram() 
        {
            return shaderProgram_;
        }
        inline VkCommandBuffer getDrawCmd() 
        {
            return drawCmd_;
        }
        inline CommandBuffer* getCommandBuffer() 
        {
            return commandBuffer_;
        }
    
    private:
        /* data */
        FrameBuffer *fbo_ = nullptr;
        VertexArrayObjectVulkan *vao_ = nullptr;
        ShaderProgram *shaderProgram_ = nullptr;
        PipelineStates *pipelineStates_ = nullptr;

        VkViewport viewport_{};
        VkRect2D scissor_{};
        std::vector<VkClearValue> clearValues_;

        VulkanContext vkCtx_;
        VkDevice device_ = VK_NULL_HANDLE;
        CommandBuffer *commandBuffer_ = nullptr;
        VkCommandBuffer drawCmd_ = VK_NULL_HANDLE;
        VkSemaphore lastPassSemaphore_ = VK_NULL_HANDLE;

        std::vector<VkSemaphore> semaphoresWait_;
        std::vector<VkSemaphore> semaphoresSignal_;
    };
}
