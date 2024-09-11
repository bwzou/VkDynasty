
#include "runtime/render/passes/MainCameraPass.h"
#include "runtime/platform/vulkan/VulkanAPI.h"
#include "runtime/render/RenderMesh.h"
#include "runtime/render/RenderResource.h"


namespace DynastyEngine 
{
    void MainCameraPass::initialize(const RenderPassInitInfo* initInfo)
    {
        LOG_INFO("MainCameraPass::initialize");
        setupAttachments();
        LOG_INFO("MainCameraPass::setupAttachments");
        setupRenderPass();
        setupDescriptorSetLayout();
        setupDescriptorSet();
        LOG_INFO("MainCameraPass::setupPipelines");
        setupPipelines();
        LOG_INFO("MainCameraPass::setupSwapchainFramebuffers");
        setupSwapchainFramebuffers();
        LOG_INFO("MainCameraPass::end initialize");
    }
    
    void MainCameraPass::draw(UIPass&           uiPass, 
                              CombineUIPass&    combineUIPass,
                              uint32_t          currentSwapchainImageIndex) 
    {   
        LOG_INFO("mFramebuffer start draw");

        VkRenderPassBeginInfo renderPassBeginInfo {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = mFramebuffer.renderPass;
        renderPassBeginInfo.framebuffer = mSwapchainFramebuffers[currentSwapchainImageIndex];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = mVulkanAPI->getSwapchainInfo().extent;

        VkClearValue clearValues[MainCameraPassAttachmentCount];
        clearValues[MainCameraPassGbufferA].color               = {{0.0f, 0.0f, 0.0f, 0.0f}};
        // clearValues[MainCameraPassGbufferB].color               = {{0.0f, 0.0f, 0.0f, 0.0f}};
        // clearValues[MainCameraPassGbufferC].color               = {{0.0f, 0.0f, 0.0f, 0.0f}};
        clearValues[MainCameraPassBackupBufferOdd].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
        // clearValues[MainCameraPassBackupBufferEven].color       = {{0.0f, 0.0f, 0.0f, 0.0f}};
        clearValues[MainCameraPassDepth].depthStencil           = {1.0f, 0};
        clearValues[MainCameraPassSwapchainImage].color         = {{0.0f, 0.5f, 0.5f, 1.0f}};
        
        renderPassBeginInfo.clearValueCount = (sizeof(clearValues) / sizeof(clearValues[0]));
        renderPassBeginInfo.pClearValues    = clearValues;

        mVulkanAPI->cmdBeginRenderPass(mVulkanAPI->getCurrentCommandBuffer(), renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        LOG_INFO("MainCameraPass::draw 1");
        drawMeshGbuffer();
        
        mVulkanAPI->cmdNextSubpass(mVulkanAPI->getCurrentCommandBuffer(), VK_SUBPASS_CONTENTS_INLINE);

        VkClearAttachment clear_attachments[1];
        clear_attachments[0].aspectMask                  = VK_IMAGE_ASPECT_COLOR_BIT;
        clear_attachments[0].colorAttachment             = 0;
        clear_attachments[0].clearValue.color.float32[0] = 0.0;
        clear_attachments[0].clearValue.color.float32[1] = 0.0;
        clear_attachments[0].clearValue.color.float32[2] = 0.0;
        clear_attachments[0].clearValue.color.float32[3] = 0.0;
        VkClearRect clear_rects[1];
        clear_rects[0].baseArrayLayer     = 0;
        clear_rects[0].layerCount         = 1;
        clear_rects[0].rect.offset.x      = 0;
        clear_rects[0].rect.offset.y      = 0;
        clear_rects[0].rect.extent.width  = mVulkanAPI->getSwapchainInfo().extent.width;
        clear_rects[0].rect.extent.height = mVulkanAPI->getSwapchainInfo().extent.height;
        mVulkanAPI->cmdClearAttachments(mVulkanAPI->getCurrentCommandBuffer(),
                                      sizeof(clear_attachments) / sizeof(clear_attachments[0]),
                                      clear_attachments,
                                      sizeof(clear_rects) / sizeof(clear_rects[0]),
                                      clear_rects);

        LOG_INFO("MainCameraPass::draw 2");
        uiPass.draw();
        
        mVulkanAPI->cmdNextSubpass(mVulkanAPI->getCurrentCommandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
        
        LOG_INFO("MainCameraPass::draw 3");
        combineUIPass.draw();

        mVulkanAPI->cmdEndRenderPass(mVulkanAPI->getCurrentCommandBuffer());

        LOG_INFO("MainCameraPass::end draw!");
    }

    void MainCameraPass::drawMeshGbuffer() 
    {
        // 默认使用第一个
        mRenderResource->updateUniformBuffer(mVulkanAPI, 0);

        mVulkanAPI->cmdBindPipeline(mVulkanAPI->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderPipelines[RenderPipelineTypeMeshGBuffer].pipeline);
        
        VkBuffer vertexBuffers[] = {mRenderResource->vertexBuffer};
        VkDeviceSize  offsets[] =  {0};
        vkCmdBindVertexBuffers(mVulkanAPI->getCurrentCommandBuffer(), 0, 1,  vertexBuffers, offsets);
        vkCmdBindIndexBuffer(mVulkanAPI->getCurrentCommandBuffer(), mRenderResource->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        mVulkanAPI->cmdSetViewport(mVulkanAPI->getCurrentCommandBuffer(), 0, 1, mVulkanAPI->getSwapchainInfo().viewport);
        mVulkanAPI->cmdSetScissor(mVulkanAPI->getCurrentCommandBuffer(), 0, 1, mVulkanAPI->getSwapchainInfo().scissor);

        vkCmdBindDescriptorSets(mVulkanAPI->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderPipelines[RenderPipelineTypeMeshGBuffer].layout, 0, 1, &mDescriptorInfos[PerMesh].descriptorSet, 0, nullptr);

        vkCmdDrawIndexed(mVulkanAPI->getCurrentCommandBuffer(), static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }

    void MainCameraPass::setupAttachments()
    {
        mFramebuffer.attachments.resize(MainCameraPassCustomAttachmentCount);

        mFramebuffer.attachments[MainCameraPassGbufferA].format          = VK_FORMAT_R8G8B8A8_UNORM;
        // mFramebuffer.attachments[MainCameraPassGbufferB].format          = VK_FORMAT_R8G8B8A8_UNORM;
        // mFramebuffer.attachments[MainCameraPassGbufferC].format          = VK_FORMAT_R8G8B8A8_SRGB;
        mFramebuffer.attachments[MainCameraPassBackupBufferOdd].format   = VK_FORMAT_R16G16B16A16_SFLOAT;
        // mFramebuffer.attachments[MainCameraPassBackupBufferEven].format  = VK_FORMAT_R16G16B16A16_SFLOAT;

        for (int index = 0; index < MainCameraPassCustomAttachmentCount; ++ index) 
        {
            if (index == MainCameraPassGbufferA)
            {
                mVulkanAPI->createImage(mVulkanAPI->getSwapchainInfo().extent.width,
                                        mVulkanAPI->getSwapchainInfo().extent.height, 
                                        mFramebuffer.attachments[MainCameraPassGbufferA].format,
                                        VK_IMAGE_TILING_OPTIMAL,
                                        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        mFramebuffer.attachments[MainCameraPassGbufferA].image,
                                        mFramebuffer.attachments[MainCameraPassGbufferA].mem,
                                        0, 
                                        1,
                                        1);
            }
            else 
            {
                mVulkanAPI->createImage(mVulkanAPI->getSwapchainInfo().extent.width,
                                        mVulkanAPI->getSwapchainInfo().extent.height,
                                        mFramebuffer.attachments[index].format,
                                        VK_IMAGE_TILING_OPTIMAL,
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        mFramebuffer.attachments[index].image,
                                        mFramebuffer.attachments[index].mem,
                                        0,
                                        1,
                                        1);
            }
            mVulkanAPI->createImageView(mFramebuffer.attachments[index].image,
                                        mFramebuffer.attachments[index].format,
                                        VK_IMAGE_ASPECT_COLOR_BIT,
                                        VK_IMAGE_VIEW_TYPE_2D,
                                        1,
                                        1,
                                        mFramebuffer.attachments[index].view);
            
        }
    }
    
    void MainCameraPass::setupRenderPass()
    {
        VkAttachmentDescription attachments[MainCameraPassAttachmentCount] = {};

        VkAttachmentDescription& gbufferNormalAttachmentDescription = attachments[MainCameraPassGbufferA];
        gbufferNormalAttachmentDescription.format         = mFramebuffer.attachments[MainCameraPassGbufferA].format;
        // gbufferNormalAttachmentDescription.format         = mVulkanAPI->getSwapchainInfo().imageFormat;
        gbufferNormalAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        gbufferNormalAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gbufferNormalAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        gbufferNormalAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gbufferNormalAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gbufferNormalAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        gbufferNormalAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // VkAttachmentDescription& gbufferMetallicRoughnessShadingmodeidAttachmentDescription = attachments[MainCameraPassGbufferB];
        // gbufferMetallicRoughnessShadingmodeidAttachmentDescription.format         = mFramebuffer.attachments[MainCameraPassGbufferB].format;
        // gbufferMetallicRoughnessShadingmodeidAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        // gbufferMetallicRoughnessShadingmodeidAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // gbufferMetallicRoughnessShadingmodeidAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // gbufferMetallicRoughnessShadingmodeidAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        // gbufferMetallicRoughnessShadingmodeidAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // gbufferMetallicRoughnessShadingmodeidAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        // gbufferMetallicRoughnessShadingmodeidAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // VkAttachmentDescription& gbufferAlbedoAttachmentDescription = attachments[MainCameraPassGbufferC];
        // gbufferAlbedoAttachmentDescription.format         = mFramebuffer.attachments[MainCameraPassGbufferC].format;
        // gbufferAlbedoAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        // gbufferAlbedoAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // gbufferAlbedoAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // gbufferAlbedoAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        // gbufferAlbedoAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // gbufferAlbedoAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        // gbufferAlbedoAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& backupOddColorAttachmentDescription = attachments[MainCameraPassBackupBufferOdd];
        backupOddColorAttachmentDescription.format         = mFramebuffer.attachments[MainCameraPassBackupBufferOdd].format;
        backupOddColorAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        backupOddColorAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        backupOddColorAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        backupOddColorAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        backupOddColorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        backupOddColorAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        backupOddColorAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // VkAttachmentDescription& backupEvenColorAttachmentDescription = attachments[MainCameraPassBackupBufferEven];
        // backupEvenColorAttachmentDescription.format         = mFramebuffer.attachments[MainCameraPassBackupBufferEven].format;
        // backupEvenColorAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        // backupEvenColorAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // backupEvenColorAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // backupEvenColorAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        // backupEvenColorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // backupEvenColorAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        // backupEvenColorAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& depthAttachmentDescription = attachments[MainCameraPassDepth];
        depthAttachmentDescription.format                   = mVulkanAPI->getDepthImageInfo().depthImageFormat;
        depthAttachmentDescription.samples                  = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentDescription.loadOp                   = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.storeOp                  = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentDescription.stencilLoadOp            = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDescription.stencilStoreOp           = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDescription.finalLayout              = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription& swapchainImageAttachmentDescription = attachments[MainCameraPassSwapchainImage];
        swapchainImageAttachmentDescription.format         = mVulkanAPI->getSwapchainInfo().imageFormat;
        swapchainImageAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        swapchainImageAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        swapchainImageAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        swapchainImageAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        swapchainImageAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        swapchainImageAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        swapchainImageAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkSubpassDescription subpasses[MainCameraSubpassCount] = {};

        VkAttachmentReference basePassColorAttachmentsReference[1] = {};
        basePassColorAttachmentsReference[0].attachment = &gbufferNormalAttachmentDescription - attachments;
        // basePassColorAttachmentsReference[0].attachment = &swapchainImageAttachmentDescription - attachments;
        basePassColorAttachmentsReference[0].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // basePassColorAttachmentsReference[1].attachment = &gbufferMetallicRoughnessShadingmodeidAttachmentDescription - attachments;
        // basePassColorAttachmentsReference[1].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // basePassColorAttachmentsReference[2].attachment = &gbufferAlbedoAttachmentDescription - attachments;
        // basePassColorAttachmentsReference[2].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference basePassDepthAttachmentReference {};
        basePassDepthAttachmentReference.attachment = &depthAttachmentDescription - attachments;
        basePassDepthAttachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& basePass = subpasses[MainCameraSubpassBasePass];
        basePass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        basePass.colorAttachmentCount    = sizeof(basePassColorAttachmentsReference) / sizeof(basePassColorAttachmentsReference[0]);
        basePass.pColorAttachments       = &basePassColorAttachmentsReference[0];
        basePass.pDepthStencilAttachment = &basePassDepthAttachmentReference;
        basePass.preserveAttachmentCount = 0;
        basePass.pPreserveAttachments    = NULL;

        VkAttachmentReference uiPassColorAttachmentReference {};
        uiPassColorAttachmentReference.attachment = &backupOddColorAttachmentDescription - attachments;
        uiPassColorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        uint32_t uiPassPreserveAttachment = &gbufferNormalAttachmentDescription - attachments;

        VkSubpassDescription& uiPass   = subpasses[MainCameraSubpassUi];
        uiPass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        uiPass.inputAttachmentCount    = 0;
        uiPass.pInputAttachments       = NULL;
        uiPass.colorAttachmentCount    = 1;
        uiPass.pColorAttachments       = &uiPassColorAttachmentReference;
        uiPass.pDepthStencilAttachment = NULL;
        uiPass.preserveAttachmentCount = 1;
        uiPass.pPreserveAttachments    = &uiPassPreserveAttachment; // 一个指向 preserveAttachmentCount 个render pass attachment的数组指针，指出当前subpass不使用，但是需要被保留的attachments。

        VkAttachmentReference combineUIPassInputAttachmentsReference[2] = {};
        combineUIPassInputAttachmentsReference[0].attachment = &gbufferNormalAttachmentDescription - attachments;
        combineUIPassInputAttachmentsReference[0].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        combineUIPassInputAttachmentsReference[1].attachment = &backupOddColorAttachmentDescription - attachments;
        combineUIPassInputAttachmentsReference[1].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference combineUIPassColorAttachmentReference {};
        combineUIPassColorAttachmentReference.attachment = &swapchainImageAttachmentDescription - attachments;
        combineUIPassColorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& combineUIPass   = subpasses[MainCameraSubpassCombineUI];
        combineUIPass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        combineUIPass.inputAttachmentCount    = sizeof(combineUIPassInputAttachmentsReference) / sizeof(combineUIPassInputAttachmentsReference[0]);
        combineUIPass.pInputAttachments       = combineUIPassInputAttachmentsReference;
        combineUIPass.colorAttachmentCount    = 1;
        combineUIPass.pColorAttachments       = &combineUIPassColorAttachmentReference;
        combineUIPass.pDepthStencilAttachment = NULL;
        combineUIPass.preserveAttachmentCount = 0;
        combineUIPass.pPreserveAttachments    = NULL;

        VkSubpassDependency dependencies[MainCameraSubpassCount] = {};
        VkSubpassDependency& mainCameraBasePass = dependencies[0];
        mainCameraBasePass.srcSubpass           = VK_SUBPASS_EXTERNAL;
        mainCameraBasePass.dstSubpass           = MainCameraSubpassBasePass;
        mainCameraBasePass.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        mainCameraBasePass.dstStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        mainCameraBasePass.srcAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        mainCameraBasePass.dstAccessMask        = VK_ACCESS_SHADER_READ_BIT;
        mainCameraBasePass.dependencyFlags      = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency& uiPassDependOnBasePass = dependencies[1];
        uiPassDependOnBasePass.srcSubpass           = MainCameraSubpassBasePass;
        uiPassDependOnBasePass.dstSubpass           = MainCameraSubpassUi;
        uiPassDependOnBasePass.srcStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        uiPassDependOnBasePass.dstStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        uiPassDependOnBasePass.srcAccessMask        = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        uiPassDependOnBasePass.dstAccessMask        = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        uiPassDependOnBasePass.dependencyFlags      = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency& combineUiPassDependOnUiPass = dependencies[2];
        combineUiPassDependOnUiPass.srcSubpass           = MainCameraSubpassUi;
        combineUiPassDependOnUiPass.dstSubpass           = MainCameraSubpassCombineUI;
        combineUiPassDependOnUiPass.srcStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        combineUiPassDependOnUiPass.dstStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        combineUiPassDependOnUiPass.srcAccessMask        = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        combineUiPassDependOnUiPass.dstAccessMask        = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        combineUiPassDependOnUiPass.dependencyFlags      = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderpassCreateInfo {};
        renderpassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpassCreateInfo.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
        renderpassCreateInfo.pAttachments    = attachments;
        renderpassCreateInfo.subpassCount    = (sizeof(subpasses) / sizeof(subpasses[0]));
        renderpassCreateInfo.pSubpasses      = subpasses;
        renderpassCreateInfo.dependencyCount = (sizeof(dependencies) / sizeof(dependencies[0]));
        renderpassCreateInfo.pDependencies   = dependencies;

        if (mVulkanAPI->createRenderPass(renderpassCreateInfo, mFramebuffer.renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass");
        }


        // VkAttachmentReference colorAttachmentRef{};     
        // colorAttachmentRef.attachment = 0;
        // colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // VkAttachmentReference depthAttachmentRef{};
        // depthAttachmentRef.attachment = 1;
        // depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // // VkAttachmentReference colorAttachmentResolveRef{};
        // // colorAttachmentResolveRef.attachment = 2;
        // // colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // VkSubpassDescription subpass{};
        // subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // subpass.colorAttachmentCount = 1;
        // subpass.pColorAttachments = &colorAttachmentRef;
        // subpass.pDepthStencilAttachment = &depthAttachmentRef;
        // subpass.pResolveAttachments = NULL;

        // VkSubpassDependency dependency{};
        // dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        // dependency.dstSubpass = 0;
        // dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        // dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        // dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        // dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        // VkRenderPassCreateInfo renderPassInfo{};
        // renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        // renderPassInfo.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
        // renderPassInfo.pAttachments = attachments;
        // renderPassInfo.subpassCount = 1;
        // renderPassInfo.pSubpasses = &subpass;
        // renderPassInfo.dependencyCount = 1;
        // renderPassInfo.pDependencies = &dependency;

        // if (mVulkanAPI->createRenderPass(renderPassInfo, mFramebuffer.renderPass) != VK_SUCCESS)
        // {
        //     throw std::runtime_error("failed to create render pass");
        // }
    }
    
    void MainCameraPass::setupDescriptorSetLayout()
    {
        // 测试专用
        mDescriptorInfos.resize(LayoutTypeCount);
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        // VkDescriptorSetLayoutBinding samplerLayoutBind{};
        // samplerLayoutBind.binding = 1;
        // samplerLayoutBind.descriptorCount = 1;
        // samplerLayoutBind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // samplerLayoutBind.pImmutableSamplers = nullptr;
        // samplerLayoutBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        // std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBind};
        // VkDescriptorSetLayoutCreateInfo layoutInfo{};
        // layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        // layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        // layoutInfo.pBindings = bindings.data();

        std::array<VkDescriptorSetLayoutBinding, 1> bindings = {uboLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(mVulkanAPI->mDevice, &layoutInfo, nullptr, &mDescriptorInfos[PerMesh].layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        // mDescriptorInfos.resize(LayoutTypeCount);
        // {
        //     VkDescriptorSetLayoutBinding meshMeshLayoutBindings[1];

        //     VkDescriptorSetLayoutBinding& meshMeshLayoutUniformBufferBinding = meshMeshLayoutBindings[0];
        //     meshMeshLayoutUniformBufferBinding.binding                       = 0;
        //     meshMeshLayoutUniformBufferBinding.descriptorType                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        //     meshMeshLayoutUniformBufferBinding.descriptorCount               = 1;
        //     meshMeshLayoutUniformBufferBinding.stageFlags                    = VK_SHADER_STAGE_VERTEX_BIT;
        //     meshMeshLayoutUniformBufferBinding.pImmutableSamplers            = NULL;

        //     VkDescriptorSetLayoutCreateInfo meshMeshLayoutCreateInfo {};
        //     meshMeshLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        //     meshMeshLayoutCreateInfo.bindingCount = 1;
        //     meshMeshLayoutCreateInfo.pBindings    = meshMeshLayoutBindings;

        //     if (mVulkanAPI->createDescriptorSetLayout(meshMeshLayoutCreateInfo, mDescriptorInfos[PerMesh].layout) != VK_SUCCESS)
        //     {
        //         throw std::runtime_error("create mesh mesh layout");
        //     }
        // }

        // {
        //     VkDescriptorSetLayoutBinding meshGlobalLayoutBindings[8];

        //     VkDescriptorSetLayoutBinding& meshGlobalLayoutPerframeStorageBufferBinding = meshGlobalLayoutBindings[0];
        //     meshGlobalLayoutPerframeStorageBufferBinding.binding            = 0;
        //     meshGlobalLayoutPerframeStorageBufferBinding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        //     meshGlobalLayoutPerframeStorageBufferBinding.descriptorCount    = 1;
        //     meshGlobalLayoutPerframeStorageBufferBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        //     meshGlobalLayoutPerframeStorageBufferBinding.pImmutableSamplers = NULL;

        //     VkDescriptorSetLayoutBinding& meshGlobalLayoutPerdrawcallStorageBufferBinding = meshGlobalLayoutBindings[1];
        //     meshGlobalLayoutPerdrawcallStorageBufferBinding.binding            = 1;
        //     meshGlobalLayoutPerdrawcallStorageBufferBinding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        //     meshGlobalLayoutPerdrawcallStorageBufferBinding.descriptorCount    = 1;
        //     meshGlobalLayoutPerdrawcallStorageBufferBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        //     meshGlobalLayoutPerdrawcallStorageBufferBinding.pImmutableSamplers = NULL;

        //     VkDescriptorSetLayoutBinding& meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding = meshGlobalLayoutBindings[2];
        //     meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.binding = 2;
        //     meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        //     meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.descriptorCount = 1;
        //     meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        //     meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.pImmutableSamplers = NULL;
        // }
    }
        
    void MainCameraPass::setupDescriptorSet()
    {
        // 测试专用
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mVulkanAPI->mDescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &mDescriptorInfos[PerMesh].layout;
        if (!mVulkanAPI->allocateDescriptorSets(allocInfo, mDescriptorInfos[PerMesh].descriptorSet)) 
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mRenderResource->uniformBuffers[0];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrites{};
        descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites.dstSet = mDescriptorInfos[PerMesh].descriptorSet;
        descriptorWrites.dstBinding = 0;
        descriptorWrites.dstArrayElement = 0;
        descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites.descriptorCount = 1;
        descriptorWrites.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(mVulkanAPI->mDevice, 1, &descriptorWrites, 0, nullptr);
    }
    
    void MainCameraPass::setupPipelines()
    {
        mRenderPipelines.resize(RenderPipelineTypeCount);
        // mesh gbuffer
        {
            VkDescriptorSetLayout      descriptorsetLayouts[1] = {mDescriptorInfos[0].layout};
            VkPipelineLayoutCreateInfo  pipelineLayoutCreateInfo {};
            pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = 1;
            pipelineLayoutCreateInfo.pSetLayouts    = descriptorsetLayouts;
            if (!mVulkanAPI->createPipelineLayout(&pipelineLayoutCreateInfo, mRenderPipelines[RenderPipelineTypeMeshGBuffer].layout))
            {
                throw std::runtime_error("create mesh gbuffer pipeline layout");
            }

            VkShaderModule vertexShader   = VK_NULL_HANDLE;
            VkShaderModule fragmentShader = VK_NULL_HANDLE;
            std::vector<VkShaderModule> module;
            module.resize(2);
            std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
            shaderStages.resize(2);
            compileGLSLFile(module, shaderStages, "mesh");

            auto vertexBindingDescriptions   = Vertex::getBindingDescription();
            auto vertexAttributeDescriptions = Vertex::getAttributeDescriptions();

            VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo {};
            vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputStateCreateInfo.vertexBindingDescriptionCount   = 1;
            vertexInputStateCreateInfo.pVertexBindingDescriptions      = &vertexBindingDescriptions;
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
            vertexInputStateCreateInfo.pVertexAttributeDescriptions    = vertexAttributeDescriptions.data();   

            VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo {};
            inputAssemblyCreateInfo.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport = mVulkanAPI->getSwapchainInfo().viewport;
            VkRect2D scissor = mVulkanAPI->getSwapchainInfo().scissor;
            VkPipelineViewportStateCreateInfo viewportStateCreateInfo {};
            viewportStateCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports    = &viewport;
            viewportStateCreateInfo.scissorCount  = 1;
            viewportStateCreateInfo.pScissors     = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo {};
            rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationStateCreateInfo.depthClampEnable        = VK_FALSE;
            rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
            rasterizationStateCreateInfo.polygonMode             = VK_POLYGON_MODE_FILL;
            rasterizationStateCreateInfo.lineWidth               = 1.0f;
            rasterizationStateCreateInfo.cullMode                = VK_CULL_MODE_BACK_BIT;
            rasterizationStateCreateInfo.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizationStateCreateInfo.depthBiasEnable         = VK_FALSE;
            rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
            rasterizationStateCreateInfo.depthBiasClamp          = 0.0f;
            rasterizationStateCreateInfo.depthBiasSlopeFactor    = 0.0f;

            VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo {};
            multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleStateCreateInfo.sampleShadingEnable  = VK_FALSE;
            multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            LOG_INFO("897");
            VkPipelineColorBlendAttachmentState colorBlendAttachments[1] = {};
            colorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachments[0].blendEnable         = VK_FALSE;
            colorBlendAttachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachments[0].colorBlendOp        = VK_BLEND_OP_ADD;
            colorBlendAttachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachments[0].alphaBlendOp        = VK_BLEND_OP_ADD;
            // colorBlendAttachments[1].colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            //                                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            // colorBlendAttachments[1].blendEnable         = VK_FALSE;
            // colorBlendAttachments[1].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            // colorBlendAttachments[1].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            // colorBlendAttachments[1].colorBlendOp        = VK_BLEND_OP_ADD;
            // colorBlendAttachments[1].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            // colorBlendAttachments[1].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            // colorBlendAttachments[1].alphaBlendOp        = VK_BLEND_OP_ADD;
            // colorBlendAttachments[2].colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            //                                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            // colorBlendAttachments[2].blendEnable         = VK_FALSE;
            // colorBlendAttachments[2].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            // colorBlendAttachments[2].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            // colorBlendAttachments[2].colorBlendOp        = VK_BLEND_OP_ADD;
            // colorBlendAttachments[2].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            // colorBlendAttachments[2].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            // colorBlendAttachments[2].alphaBlendOp        = VK_BLEND_OP_ADD;                                                 

            VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
            colorBlendStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendStateCreateInfo.logicOpEnable     = VK_FALSE;
            colorBlendStateCreateInfo.logicOp           = VK_LOGIC_OP_COPY;
            colorBlendStateCreateInfo.attachmentCount   = 1;
            colorBlendStateCreateInfo.pAttachments      = &colorBlendAttachments[0];
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

            VkDynamicState                   dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

            VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {};
            dynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateCreateInfo.dynamicStateCount = 2;
            dynamicStateCreateInfo.pDynamicStates    = dynamicStates;

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
            pipelineInfo.layout              = mRenderPipelines[RenderPipelineTypeMeshGBuffer].layout;
            pipelineInfo.renderPass          = mFramebuffer.renderPass;
            pipelineInfo.subpass             = MainCameraSubpassBasePass;
            pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState       = &dynamicStateCreateInfo;
            if (!mVulkanAPI->createGraphicsPipelines(VK_NULL_HANDLE, 1, pipelineInfo, mRenderPipelines[RenderPipelineTypeMeshGBuffer].pipeline))
            {
                throw std::runtime_error("create mesh gbuffer graphics pipeline");
            }

            mVulkanAPI->destroyShaderModule(module[0]);
            mVulkanAPI->destroyShaderModule(module[1]);
        }

        // mesh lighting
        
        // deferred lighting
        
        // draw axis

    }
    
    void MainCameraPass::setupSwapchainFramebuffers()
    {
        mSwapchainFramebuffers.resize(mVulkanAPI->getSwapchainInfo().imageViews.size());
        for (size_t i = 0; i < mVulkanAPI->getSwapchainInfo().imageViews.size(); i++) 
        {
            std::vector<VkImageView> framebufferAttachmentsForImageView = 
            {
                mFramebuffer.attachments[MainCameraPassGbufferA].view,
                // mFramebuffer.attachments[MainCameraPassGbufferB].view,
                // mFramebuffer.attachments[MainCameraPassGbufferC].view,
                mFramebuffer.attachments[MainCameraPassBackupBufferOdd].view,
                // mFramebuffer.attachments[MainCameraPassBackupBufferEven].view,
                // mFramebuffer.attachments[_main_camera_pass_post_process_buffer_odd].view,
                // mFramebuffer.attachments[_main_camera_pass_post_process_buffer_even].view,    
                
                mVulkanAPI->getDepthImageInfo().depthImageView,
                mVulkanAPI->getSwapchainInfo().imageViews[i]
            };
            
            // FramebufferCreateInfo framebufferCreateInfo {};
            // framebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            // framebufferCreateInfo.flags           = 0U;
            // framebufferCreateInfo.renderPass      = &mFramebuffer.renderPass;
            // framebufferCreateInfo.attachmentCount = framebufferAttachmentsForImageView.size();
            // framebufferCreateInfo.pAttachments    = framebufferAttachmentsForImageView;
            // framebufferCreateInfo.width           = mVulkanAPI->getSwapchainInfo().extent.width;
            // framebufferCreateInfo.height          = mVulkanAPI->getSwapchainInfo().extent.height;
            // framebufferCreateInfo.layers          = 1;

            // if (VK_SUCCESS != mVulkanAPI->createFramebuffer(&framebufferCreateInfo, mSwapchainFramebuffers[i]))
            // {
            //     throw std::runtime_error("create main camera framebuffer");
            // }

            VkFramebufferCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.pNext = NULL;
            createInfo.flags = 0U;
            createInfo.renderPass = mFramebuffer.renderPass;
            createInfo.attachmentCount = static_cast<uint32_t>(framebufferAttachmentsForImageView.size());
            createInfo.pAttachments = framebufferAttachmentsForImageView.data();
            createInfo.width = mVulkanAPI->getSwapchainInfo().extent.width;
            createInfo.height = mVulkanAPI->getSwapchainInfo().extent.height;
            createInfo.layers = 1;

            VkResult result = vkCreateFramebuffer(mVulkanAPI->mDevice, &createInfo, nullptr, &mSwapchainFramebuffers[i]);
        }
    }

    void MainCameraPass::updateAfterFramebufferRecreate()
    {
        LOG_INFO("MainCameraPass::updateAfterFramebufferRecreate");
        for (size_t i = 0; i < mFramebuffer.attachments.size(); i++)
        {
            mVulkanAPI->destroyImage(mFramebuffer.attachments[i].image);
            mVulkanAPI->destroyImageView(mFramebuffer.attachments[i].view);
            mVulkanAPI->freeMemory(mFramebuffer.attachments[i].mem);
        }

        for (auto framebuffer : mSwapchainFramebuffers)
        {
            mVulkanAPI->destroyFramebuffer(framebuffer);
        }

        setupAttachments();
        setupDescriptorSet();
        setupSwapchainFramebuffers();
    }       
}