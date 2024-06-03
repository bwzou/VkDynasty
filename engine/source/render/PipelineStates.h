#pragma once

#include "VulkanContext.h"
#include "RenderStates.h"
#include "EnumsVulkan.h"
#include "ShaderProgram.h"
#include "../code/util/HashUtils.h"

struct PipelineContainer {
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;
};

class PipelineStates {
public:
    explicit PipelineStates(VulkanContext &ctx, const RenderStates &states) 
        : vkCtx_(ctx), renderStates(states) {
            device_ = ctx.device();
        }

    ~PipelineStates() {

    }

    void create(VkPipelineVertexInputStateCreateInfo &vertexInputInfo,
                            ShaderProgram *program,
                            VkRenderPass &renderPass,
                            VkSampleCountFlagBits sampleCount);
    PipelineContainer createGraphicsPipeline(VkPipelineVertexInputStateCreateInfo &vertexInputInfo,
                                ShaderProgram *program,
                                VkRenderPass &renderPass,
                                VkSampleCountFlagBits sampleCount);

    static size_t getPipelineCacheKey(ShaderProgram *program,
                                    VkRenderPass &renderPass,
                                    VkSampleCountFlagBits sampleCount) {
        size_t seed = 0;
        HashUtils::hashCombine(seed, (void *) program);
        HashUtils::hashCombine(seed, (void *) renderPass);
        HashUtils::hashCombine(seed, (uint32_t) sampleCount);
    }                            

    VkPipeline getGraphicsPipeline() const{
        return currPipeline_.graphicsPipeline_;
    }

    VkPipelineLayout getGraphicsPipelineLayout() const {
        return currPipeline_.pipelineLayout_;
    }

public:
    VulkanContext &vkCtx_;
    VkDevice device_ = VK_NULL_HANDLE;

    PipelineContainer currPipeline_{};
    std::unordered_map<size_t, PipelineContainer> pipelineCache_;

    RenderStates renderStates;
};