#pragma once

#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include "json11.hpp"

#include "VulkanUtils.h"
#include "../function/Config.h"
#include "../function/Uniform.h"
#include "../code/base/UUID.h"
#include "../code/util/FileUtils.h"
#include "../code/base/macro.h"

#define DESCRIPTOR_SET_POOL_MAX_SIZE 64


namespace DynastyEngine
{
    enum ShaderStage 
    {
        ShaderStage_Vertex,
        ShaderStage_Fragment,
    };

    enum ShaderUniformType {
        UniformType_Unknown = 0,
        UniformType_Sampler = 1,
        UniformType_Block = 2,
    };

    struct ShaderUniformDesc 
    {
        std::string name;
        ShaderUniformType type = UniformType_Unknown;
        int location = 0;
        int binding = 0;
        int set = 0;
    };

    struct UniformInfo {
        ShaderUniformType type;
        uint32_t binding;
        VkShaderStageFlags stageFlags;
    };

    class ShaderProgram 
    {
    public:
        explicit ShaderProgram(VulkanContext &ctx) : vkCtx_(ctx) 
        {
            device_ = ctx.device();
            glslHeader_ = "#version 450\n";
        }
        ~ShaderProgram() 
        {
            vkDestroyShaderModule(device_, vertexShader_, nullptr);
            vkDestroyShaderModule(device_, fragmentShader_, nullptr);

            vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
        }

        bool compileAndLinkGLSLFile(const std::string &vsPath, const std::string &fsPath, const std::string source);
        bool compileAndLinkGLSL(const std::vector<char> &vsSource, 
                                std::unordered_map<std::string, ShaderUniformDesc> uniformsDescVertex, 
                                const std::vector<char> &fsSource,
                                std::unordered_map<std::string, ShaderUniformDesc> uniformsDescFragment);
        std::unordered_map<std::string, ShaderUniformDesc> readFromFile(std::string jsonFilePath);

        void createDescriptorSetLayouts(std::unordered_map<std::string, ShaderUniformDesc> &uniformsDescVertex,
                                        std::unordered_map<std::string, ShaderUniformDesc> &uniformsDescFragment);
        void createDescriptorPool();
        DescriptorSet *getNewDescriptorSet(VkDescriptorSetLayout layout);
        void createShaderStages();
        void createShaderModule(VkShaderModule  &shaderModule, const std::vector<char>& code);

        static VkDescriptorType getDescriptorType(ShaderUniformType type) 
        {
            switch (type) 
            {
                case UniformType_Block:
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                case UniformType_Sampler:
                    return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                default:
                    break;
            }
            return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }

        void bindResources(ShaderResources &resources) 
        {
            // bind resources
            for (auto &kv : resources.blocks) 
            {
                bindUniform(*kv.second);
            }
            for (auto &kv : resources.samplers) 
            {
                bindUniform(*kv.second);
            }
        }

        void bindUniformBuffer(VkDescriptorBufferInfo &info, uint32_t binding) 
        {
            VkWriteDescriptorSet writeDesc{};
            // 前两个字段指定要更新的描述符集和绑定。
            writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDesc.dstSet = descriptorSets_[0]->set; // all uniforms bind to set 0
            // 统一的缓冲区绑定索引0
            writeDesc.dstBinding = binding;
            writeDesc.dstArrayElement = 0;
            // 指定描述符的类型。可以一次更新数组中的多个描述符
            writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDesc.descriptorCount = 1;
            // 该pBufferInfo字段用于引用缓冲区数据的描述符
            writeDesc.pBufferInfo = &info;
            writeDescriptorSets_.push_back(writeDesc);
        }

        void bindUniformSampler(VkDescriptorImageInfo &info, uint32_t binding) 
        {
            VkWriteDescriptorSet writeDesc{};
            // 前两个字段指定要更新的描述符集和绑定。
            writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDesc.dstSet = descriptorSets_[0]->set; // all uniforms bind to set 0
            // 统一的缓冲区绑定索引0
            writeDesc.dstBinding = binding;
            writeDesc.dstArrayElement = 0;
            // 指定描述符的类型。可以一次更新数组中的多个描述符
            writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDesc.descriptorCount = 1;
            // 该pBufferInfo字段用于引用缓冲区数据的描述符
            writeDesc.pImageInfo = &info;
            writeDescriptorSets_.push_back(writeDesc);
        }

        void beginBindUniforms(CommandBuffer *cmd) 
        {
            currCmdBuffer_ = cmd;

            // allocate descriptor sets
            descriptorSets_.resize(descriptorSetLayouts_.size());
            for (size_t i = 0; i < descriptorSetLayouts_.size(); i++) 
            {
                descriptorSets_[i] = getNewDescriptorSet(descriptorSetLayouts_[i]);
            }
            
            writeDescriptorSets_.clear();
        }

        void endBindUniforms() 
        {
            if (!writeDescriptorSets_.empty()) 
            {
                vkUpdateDescriptorSets(device_, writeDescriptorSets_.size(), writeDescriptorSets_.data(), 0, nullptr);
            }

            // descriptorSets_ -> vkDescriptorSets_
            vkDescriptorSets_.resize(descriptorSets_.size());
            for (size_t i = 0; i < descriptorSets_.size(); i++) 
            {
                vkDescriptorSets_[i] = descriptorSets_[i]->set;
            }

            // add descriptor sets to command buffer
            currCmdBuffer_->descriptorSets.insert(currCmdBuffer_->descriptorSets.begin(), descriptorSets_.begin(), descriptorSets_.end());
        }


        bool bindUniform(Uniform &uniform) 
        {
            int hash = uniform.getHash();
            int location = -1;
            if (uniformLocations_.find(hash) == uniformLocations_.end()) 
            {
                location = uniform.getLocation(*this);
                uniformLocations_[hash] = location;
            } 
            else 
            {
                location = uniformLocations_[hash];
            }

            if (location < 0) 
            {
                return false;
            }

            uniform.bindProgram(*this, location);
            return true;
        }
        
        inline std::vector<VkPipelineShaderStageCreateInfo> &getShaderStages() 
        {
            return shaderStages_;
        }

        inline std::vector<VkDescriptorSetLayout> &getDescriptorSetLayouts() 
        {
            return descriptorSetLayouts_;
        }

        inline std::vector<VkDescriptorSet> &getDescriptorSet() 
        {
            return vkDescriptorSets_;
        }

        inline int getUniformLocation(const std::string &name) 
        {
            auto it = uniformsInfo_.find(name);
            if (it != uniformsInfo_.end()) 
            {
                return (int) it->second.binding;
            }

            return -1;
        }

        inline CommandBuffer *getCommandBuffer() 
        {
            return currCmdBuffer_;
        }
        
        inline VkDescriptorPool getDescriptorPool() 
        {
            return descriptorPool_;
        }


    private: 
        VulkanContext &vkCtx_;
        VkDevice device_ = VK_NULL_HANDLE;

        std::string glslHeader_;
        std::vector<std::string> glslDefines_;

        VkShaderModule vertexShader_ = VK_NULL_HANDLE;
        VkShaderModule fragmentShader_ = VK_NULL_HANDLE;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages_;
        std::unordered_map<std::string, UniformInfo> uniformsInfo_;

        VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts_;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets_;

        std::vector<DescriptorSet *> descriptorSets_;
        std::vector<VkDescriptorSet> vkDescriptorSets_;

        // descriptor set bool
        std::unordered_map<VkDescriptorSetLayout, std::vector<DescriptorSet>> descriptorSetPool_;
        size_t maxDescriptorSetPoolSize_ = 0;

        CommandBuffer *currCmdBuffer_ = nullptr;

        std::unordered_map<int, int> uniformLocations_;
    };
}