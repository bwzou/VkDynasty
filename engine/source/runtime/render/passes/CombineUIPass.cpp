#include "runtime/render/passes/CombineUIPass.h"
#include "runtime/platform/vulkan/VulkanAPI.h"

namespace DynastyEngine
{
    void CombineUIPass::initialize(const RenderPassInitInfo* initInfo)
    {
        const CombineUIPassInitInfo* _initInfo = static_cast<const CombineUIPassInitInfo*>(initInfo);
        mFramebuffer.renderPass = _initInfo->renderPass;

        setupDescriptorSetLayout();
        setupPipelines();
        setupDescriptorSet();
        updateAfterFramebufferRecreate(_initInfo->sceneInputAttachment, _initInfo->uiInputAttachment);
        LOG_INFO("CombineUIPass initialize end");
    }
    
    void CombineUIPass::draw()
    {
        LOG_INFO("CombineUIPass:: start draw");
        // float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

        mVulkanAPI->cmdBindPipeline(mVulkanAPI->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderPipelines[0].pipeline);

        VkViewport viewport = {0.0,
                               0.0,
                               static_cast<float>(mVulkanAPI->getSwapchainInfo().extent.width),
                               static_cast<float>(mVulkanAPI->getSwapchainInfo().extent.height),
                               0.0,
                               1.0};
        VkRect2D   scissor  = {0, 0, mVulkanAPI->getSwapchainInfo().extent.width, mVulkanAPI->getSwapchainInfo().extent.height};
        mVulkanAPI->cmdSetViewport(mVulkanAPI->getCurrentCommandBuffer(), 0, 1, viewport);
        mVulkanAPI->cmdSetScissor(mVulkanAPI->getCurrentCommandBuffer(), 0, 1, scissor);
        vkCmdBindDescriptorSets(mVulkanAPI->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderPipelines[0].layout, 0, 1, &mDescriptorInfos[0].descriptorSet, 0, nullptr);
        mVulkanAPI->cmdDraw(mVulkanAPI->getCurrentCommandBuffer(), 3, 1, 0, 0);
        
        LOG_INFO("CombineUIPass::end draw");
    }
    
    void CombineUIPass::updateAfterFramebufferRecreate(VkImageView sceneInputAttachment, VkImageView uiInputAttachment)
    {
        VkDescriptorImageInfo perFrameSceneInputAttachmentInfo = {};
        perFrameSceneInputAttachmentInfo.sampler = mVulkanAPI->getOrCreateDefaultSampler(1);
        perFrameSceneInputAttachmentInfo.imageView   = sceneInputAttachment;
        perFrameSceneInputAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo perFrameUiInputAttachmentInfo = {};
        perFrameUiInputAttachmentInfo.sampler = mVulkanAPI->getOrCreateDefaultSampler(1);
        perFrameUiInputAttachmentInfo.imageView   = uiInputAttachment;
        perFrameUiInputAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet postProcessDescriptorWritesInfo[2];
        
        VkWriteDescriptorSet& perFrameSceneInputAttachmentWriteInfo = postProcessDescriptorWritesInfo[0];
        perFrameSceneInputAttachmentWriteInfo.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        perFrameSceneInputAttachmentWriteInfo.pNext                 = NULL;
        perFrameSceneInputAttachmentWriteInfo.dstSet                = mDescriptorInfos[0].descriptorSet;
        perFrameSceneInputAttachmentWriteInfo.dstBinding            = 0;
        perFrameSceneInputAttachmentWriteInfo.dstArrayElement       = 0;
        perFrameSceneInputAttachmentWriteInfo.descriptorType        = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        perFrameSceneInputAttachmentWriteInfo.descriptorCount       = 1;
        perFrameSceneInputAttachmentWriteInfo.pImageInfo            = &perFrameSceneInputAttachmentInfo;

        VkWriteDescriptorSet& perFrameUiInputAttachmentWriteInfo = postProcessDescriptorWritesInfo[1];
        perFrameUiInputAttachmentWriteInfo.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        perFrameUiInputAttachmentWriteInfo.pNext                 = NULL;
        perFrameUiInputAttachmentWriteInfo.dstSet                = mDescriptorInfos[0].descriptorSet;
        perFrameUiInputAttachmentWriteInfo.dstBinding            = 1;
        perFrameUiInputAttachmentWriteInfo.dstArrayElement       = 0;
        perFrameUiInputAttachmentWriteInfo.descriptorType        = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        perFrameUiInputAttachmentWriteInfo.descriptorCount       = 1;
        perFrameUiInputAttachmentWriteInfo.pImageInfo            = &perFrameUiInputAttachmentInfo;

        vkUpdateDescriptorSets(mVulkanAPI->mDevice, sizeof(postProcessDescriptorWritesInfo) / sizeof(postProcessDescriptorWritesInfo[0]), postProcessDescriptorWritesInfo, 0, NULL);
    }

    void CombineUIPass::setupDescriptorSetLayout()
    {
        mDescriptorInfos.resize(1);
        VkDescriptorSetLayoutBinding postProcessGlobalLayoutBindings[2] = {};

        VkDescriptorSetLayoutBinding& globalLayoutSceneInputAttachmentBinding = postProcessGlobalLayoutBindings[0];
        globalLayoutSceneInputAttachmentBinding.binding         = 0;
        globalLayoutSceneInputAttachmentBinding.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        globalLayoutSceneInputAttachmentBinding.descriptorCount = 1;
        globalLayoutSceneInputAttachmentBinding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding& globalLayoutNormalInputAttachmentBinding = postProcessGlobalLayoutBindings[1];
        globalLayoutNormalInputAttachmentBinding.binding         = 1;
        globalLayoutNormalInputAttachmentBinding.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        globalLayoutNormalInputAttachmentBinding.descriptorCount = 1;
        globalLayoutNormalInputAttachmentBinding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo postProcessGlobalLayoutCreateInfo;
        postProcessGlobalLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        postProcessGlobalLayoutCreateInfo.pNext        = NULL;
        postProcessGlobalLayoutCreateInfo.flags        = 0;
        postProcessGlobalLayoutCreateInfo.bindingCount = sizeof(postProcessGlobalLayoutBindings) / sizeof(postProcessGlobalLayoutBindings[0]);
        postProcessGlobalLayoutCreateInfo.pBindings    = postProcessGlobalLayoutBindings;

        if (VK_SUCCESS != mVulkanAPI->createDescriptorSetLayout(postProcessGlobalLayoutCreateInfo, mDescriptorInfos[0].layout))
        {
            throw std::runtime_error("create combine ui global layout");
        }
    }
    
    void CombineUIPass::setupPipelines()
    {
        LOG_INFO("CombineUIPass::setupPipelines")
        mRenderPipelines.resize(1);

        VkDescriptorSetLayout      descriptorsetLayouts[1] = {mDescriptorInfos[0].layout};
        VkPipelineLayoutCreateInfo  pipelineLayoutCreateInfo {};
        pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts    = descriptorsetLayouts;
        if (!mVulkanAPI->createPipelineLayout(&pipelineLayoutCreateInfo, mRenderPipelines[0].layout))
        {
            throw std::runtime_error("create combine ui pipeline layout");
        }

        VkShaderModule vertexShader   = VK_NULL_HANDLE;
        VkShaderModule fragmentShader = VK_NULL_HANDLE;
        std::vector<VkShaderModule> module;
        module.resize(2);
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        shaderStages.resize(2);
        compileGLSLFile(module, shaderStages, "combine_ui");

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo {};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount   = 0;
        vertexInputStateCreateInfo.pVertexBindingDescriptions      = NULL;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions    = NULL;

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo {};
        inputAssemblyCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo {};
        VkViewport viewport = mVulkanAPI->getSwapchainInfo().viewport;
        VkRect2D  scissor   = mVulkanAPI->getSwapchainInfo().scissor;
        viewportStateCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports    = &viewport;
        viewportStateCreateInfo.scissorCount  = 1;
        viewportStateCreateInfo.pScissors     = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo {};
        rasterizationStateCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.depthClampEnable        = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.lineWidth               = 1.0f;
        rasterizationStateCreateInfo.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterizationStateCreateInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateCreateInfo.depthBiasEnable         = VK_FALSE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationStateCreateInfo.depthBiasClamp          = 0.0f;
        rasterizationStateCreateInfo.depthBiasSlopeFactor    = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo {};
        multisampleStateCreateInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.sampleShadingEnable  = VK_FALSE;
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState {};
        colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentState.blendEnable         = VK_FALSE;
        colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachmentState.alphaBlendOp        = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo {};
        colorBlendStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable     = VK_FALSE;
        colorBlendStateCreateInfo.logicOp           = VK_LOGIC_OP_COPY;
        colorBlendStateCreateInfo.attachmentCount   = 1;
        colorBlendStateCreateInfo.pAttachments      = &colorBlendAttachmentState;
        colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo {};
        depthStencilCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilCreateInfo.depthTestEnable       = VK_TRUE;
        depthStencilCreateInfo.depthWriteEnable      = VK_TRUE;
        depthStencilCreateInfo.depthCompareOp        = VK_COMPARE_OP_LESS;
        depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilCreateInfo.stencilTestEnable     = VK_FALSE;

        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {};
        dynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = 2;
        dynamicStateCreateInfo.pDynamicStates    = dynamicStates.data();

        VkGraphicsPipelineCreateInfo pipelineInfo {};
        pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shaderStages.data();
        pipelineInfo.pVertexInputState   = &vertexInputStateCreateInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        pipelineInfo.pViewportState      = &viewportStateCreateInfo;
        pipelineInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineInfo.pMultisampleState   = &multisampleStateCreateInfo;
        pipelineInfo.pColorBlendState    = &colorBlendStateCreateInfo;
        pipelineInfo.pDepthStencilState  = &depthStencilCreateInfo;
        pipelineInfo.layout              = mRenderPipelines[0].layout;
        pipelineInfo.renderPass          = mFramebuffer.renderPass;
        pipelineInfo.subpass             = MainCameraSubpassCombineUI;
        //  pipelineInfo.subpass             = 0;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipelineInfo.pDynamicState       = &dynamicStateCreateInfo;

        if (!mVulkanAPI->createGraphicsPipelines(VK_NULL_HANDLE, 1, pipelineInfo, mRenderPipelines[0].pipeline))
        {
            throw std::runtime_error("create post process graphics pipeline");
        }

        mVulkanAPI->destroyShaderModule(module[0]);
        mVulkanAPI->destroyShaderModule(module[1]);
    }
    
    void CombineUIPass::setupDescriptorSet()
    {
        VkDescriptorSetAllocateInfo postProcessGlobalDescriptorSetAllocInfo;
        postProcessGlobalDescriptorSetAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        postProcessGlobalDescriptorSetAllocInfo.pNext              = NULL;
        postProcessGlobalDescriptorSetAllocInfo.descriptorPool     = mVulkanAPI->mDescriptorPool;
        postProcessGlobalDescriptorSetAllocInfo.descriptorSetCount = 1;
        postProcessGlobalDescriptorSetAllocInfo.pSetLayouts        = &mDescriptorInfos[0].layout;

        if (!mVulkanAPI->allocateDescriptorSets(postProcessGlobalDescriptorSetAllocInfo, mDescriptorInfos[0].descriptorSet))
        {
            throw std::runtime_error("allocate post process global descriptor set");
        }
    }
}