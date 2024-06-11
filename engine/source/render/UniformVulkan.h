#pragma once

#include "../function/Uniform.h"
#include "VulKanContext.h"
#include "ShaderProgram.h"


class UniformBlockVulkan : public UniformBlock {
public:
    UniformBlockVulkan(VulkanContext &ctx, const std::string &name, int size) : vkCtx_(ctx), UniformBlock(name, size) {
        device_ = ctx.device();

        descriptorBufferInfo_.offset = 0;
        descriptorBufferInfo_.range = size;
    }

    int getLocation(ShaderProgram &program) {
        auto shaderProgram = dynamic_cast<ShaderProgram *>(&program);
        return shaderProgram->getUniformLocation(name);
    }

    void bindProgram(ShaderProgram &program, int location) {
        auto shaderProgram = dynamic_cast<ShaderProgram *>(&program);
        if (ubo_ == nullptr) {
            throw std::runtime_error("uniform bind program error: data not set");
            return;
        }

        auto *cmd = shaderProgram->getCommandBuffer();
        if (cmd  == nullptr) {
            throw std::runtime_error("uniform bind program error: not in render pass scope");
            return;
        }

        cmd->uniformBuffers.push_back(ubo_);
        shaderProgram->bindUniformBuffer(descriptorBufferInfo_, location);
        bindToCmd_ = true;
    }

    void setSubData(void *data, int len, int offset) {
        if (bindToCmd_) {
            ubo_ = vkCtx_.getNewUniformBuffer(blockSize);
            descriptorBufferInfo_.buffer = ubo_->buffer.buffer;
        }
        memcpy((uint8_t *)ubo_->mapPtr + offset, data, len);
        bindToCmd_ = false;
    }

    void setData(void *data, int len)  { 
        setSubData(data, len, 0);
    }


private:
    VulkanContext &vkCtx_;
    VkDevice device_ = VK_NULL_HANDLE;
    
    bool bindToCmd_ = true;
    UniformBuffer *ubo_ = nullptr;
    VkDescriptorBufferInfo descriptorBufferInfo_{};
};


class UniformSamplerVulkan : public UniformSampler {
public:
    UniformSamplerVulkan(VulkanContext &ctx, const std::string &name, TextureType type, TextureFormat format) : vkCtx_(ctx), UniformSampler(name, type, format) {
        device_ = ctx.device();
        vkDescriptorImageInfo_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    
    int getLocation(ShaderProgram &program) {
        auto shaderProgram = dynamic_cast<ShaderProgram *>(&program);
        return shaderProgram->getUniformLocation(name);
    }

    void bindProgram(ShaderProgram &program, int location) {
        auto shaderProgram = dynamic_cast<ShaderProgram *>(&program);
        shaderProgram->bindUniformSampler(vkDescriptorImageInfo_, location);
    }

    void setTexture(const std::shared_ptr<Texture> &tex)  { 
        auto *texVulkan = dynamic_cast<TextureVulkan *>(tex.get());
        vkDescriptorImageInfo_.imageView = texVulkan->getSampleImageView();
        vkDescriptorImageInfo_.sampler = texVulkan->getSampler();
    }


private:
    VulkanContext vkCtx_;
    VkDevice device_ = VK_NULL_HANDLE;

    VkDescriptorImageInfo vkDescriptorImageInfo_{};
};