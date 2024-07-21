#include <vulkan/vulkan.h>
#include "FrameBuffer.h"


namespace DynastyEngine
{
    bool FrameBuffer::createRenderPass() 
    {
        // // attachment参数通过附件描述数组中的索引指定要引用的附件
        // VkAttachmentReference colorAttachmentRef{};
        // colorAttachmentRef.attachment = VK_ATTACHMENT_UNUSED;
        // colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // VkAttachmentReference depthAttachmentRef{};
        // depthAttachmentRef.attachment = VK_ATTACHMENT_UNUSED;
        // depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // VkAttachmentReference resolveAttachmentRef{};
        // resolveAttachmentRef.attachment = VK_ATTACHMENT_UNUSED;
        // resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // std::vector<VkAttachmentDescription> attachments;
        // // if (!isOffscreen()) {
        //     // auto *colorTex = getAttachmentColor();

        //     // 只有一个单一的颜色缓冲区附件，由交换链中的一个图像代表
        //     VkAttachmentDescription colorAttachment{};
        //     // 颜色附件的format应该与交换链图像的格式相匹配
        //     colorAttachment.format = vkCtx_.swapChainImageFormat();
        //     colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        //     colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        //     colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            
        //     // 这里采用存储操作。
        //     // stencilLoadOp / stencilStoreOp 适用于模板数据
        //     colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //     colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //     colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //     colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        //     colorAttachmentRef.attachment = attachments.size();
        //     attachments.push_back(colorAttachment);
        // // }

        // if (colorReady_) {
        //     auto *colorTex = getAttachmentColor();

        //     // 只有一个单一的颜色缓冲区附件，由交换链中的一个图像代表
        //     VkAttachmentDescription colorAttachment{};
        //     // 颜色附件的format应该与交换链图像的格式相匹配
        //     colorAttachment.format = cvtImageFormat(colorTex->format, colorTex->usage);
        //     colorAttachment.samples = getAttachmentColor()->getSampleCount();
        //     colorAttachment.loadOp = clearStates_.colorFlag ? VK_ATTACHMENT_LOAD_OP_CLEAR : (colorTex->multiSample ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : VK_ATTACHMENT_LOAD_OP_LOAD);
        //     colorAttachment.storeOp = colorTex->multiSample > VK_SAMPLE_COUNT_1_BIT ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
            
        //     // 这里采用存储操作。
        //     // stencilLoadOp / stencilStoreOp 适用于模板数据
        //     colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //     colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //     colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //     colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        //     colorAttachmentRef.attachment = attachments.size();
        //     attachments.push_back(colorAttachment);
        // }

        // if (depthReady_) 
        // {
        //     auto *depthTex = getAttachmentDepth();

        //     VkAttachmentDescription depthAttachment{};
        //     depthAttachment.format = cvtImageFormat(depthTex->format, depthTex->usage);
        //     depthAttachment.samples = getAttachmentDepth()->getSampleCount();
        //     depthAttachment.loadOp = clearStates_.depthFlag ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        //     depthAttachment.storeOp = colorReady_ ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
        //     depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //     depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //     depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //     depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        //     depthAttachmentRef.attachment = attachments.size();
        //     attachments.push_back(depthAttachment);
        // }

        // if (colorReady_) {
        //     auto *colorTex = getAttachmentColor();
        //     // color resolve
        //     if (colorTex->multiSample) {
        //         VkAttachmentDescription colorResolveAttachment{};
        //         colorResolveAttachment.format = cvtImageFormat(colorTex->format, colorTex->usage);
        //         colorResolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        //         colorResolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //         colorResolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        //         colorResolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //         colorResolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //         colorResolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //         colorResolveAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        //         resolveAttachmentRef.attachment = attachments.size();
        //         attachments.push_back(colorResolveAttachment);
        //     }
        // }

        // 子通道
        // VkSubpassDescription subpass{};
        // subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // // 明确指出这是一个图形子通道
        // subpass.colorAttachmentCount = 1;
        // subpass.pColorAttachments = &colorAttachmentRef;
        // subpass.pDepthStencilAttachment = &depthAttachmentRef;
        // subpass.pResolveAttachments = &resolveAttachmentRef;

        // VkRenderPassCreateInfo renderPassInfo{};
        // renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        // renderPassInfo.attachmentCount = attachments.size();
        // renderPassInfo.pAttachments = attachments.data();
        // renderPassInfo.subpassCount = 1;
        // renderPassInfo.pSubpasses = &subpass;
        // renderPassInfo.dependencyCount = 0;
        
        // renderPassInfo.dependencyCount = 1;
        // renderPassInfo.pDependencies = &dependency;

        VkAttachmentDescription attachments[MainCameraPassAttachmentCount] = {};

        VkAttachmentDescription& swapchainImageAttachmentDescription = attachments[MainCameraPassSwapchainImage];
        swapchainImageAttachmentDescription.format         = vkCtx_.swapChainImageFormat();
        swapchainImageAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        swapchainImageAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        swapchainImageAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        swapchainImageAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        swapchainImageAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        swapchainImageAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        swapchainImageAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        auto *depthTex = getAttachmentDepth();
        VkAttachmentDescription& depthAttachmentDescription = attachments[MainCameraPassDepth];
        depthAttachmentDescription.format                   = cvtImageFormat(depthTex->format, depthTex->usage);
        depthAttachmentDescription.samples                  = getAttachmentDepth()->getSampleCount();
        depthAttachmentDescription.loadOp                   = clearStates_.depthFlag ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;;
        depthAttachmentDescription.storeOp                  = colorReady_ ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;;
        depthAttachmentDescription.stencilLoadOp            = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDescription.stencilStoreOp           = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDescription.finalLayout              = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


        VkAttachmentReference basePassColorAttachmentReference{};
        basePassColorAttachmentReference.attachment = 0;
        basePassColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference basePassDepthAttachmentReference{};
        basePassDepthAttachmentReference.attachment = 1;
        basePassDepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference basePassResolveAttachmentReference{};
        basePassResolveAttachmentReference.attachment = VK_ATTACHMENT_UNUSED;
        basePassResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 子通道
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // 明确指出这是一个图形子通道
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &basePassColorAttachmentReference;
        subpass.pDepthStencilAttachment = &basePassDepthAttachmentReference;
        subpass.pResolveAttachments = &basePassResolveAttachmentReference;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
        renderPassInfo.pAttachments = attachments;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 0;


        // VkAttachmentDescription& backupEvenColorAttachmentDescription = attachments[MainCameraPassBackupBufferEven];
        // backupEvenColorAttachmentDescription.format         =  VK_FORMAT_R16G16B16A16_SFLOAT;
        // backupEvenColorAttachmentDescription.samples        =  VK_SAMPLE_COUNT_1_BIT;
        // backupEvenColorAttachmentDescription.loadOp         =  VK_ATTACHMENT_LOAD_OP_CLEAR;
        // backupEvenColorAttachmentDescription.storeOp        =  VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // backupEvenColorAttachmentDescription.stencilLoadOp  =  VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        // backupEvenColorAttachmentDescription.stencilStoreOp =  VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // backupEvenColorAttachmentDescription.initialLayout  =  VK_IMAGE_LAYOUT_UNDEFINED;
        // backupEvenColorAttachmentDescription.finalLayout    =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // VkAttachmentDescription& backupOddColorAttachmentDescription = attachments[MainCameraPassBackupBufferOdd];
        // backupOddColorAttachmentDescription.format         = VK_FORMAT_R16G16B16A16_SFLOAT;
        // backupOddColorAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        // backupOddColorAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // backupOddColorAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // backupOddColorAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        // backupOddColorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // backupOddColorAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        // backupOddColorAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        
        // 子通道
        // VkSubpassDescription subpasses[MainCameraSubpassCount] = {};

        // // attachment参数通过附件描述数组中的索引指定要引用的附件
        // VkAttachmentReference basePassColorAttachmentReference{};
        // basePassColorAttachmentReference.attachment = 0;
        // basePassColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // VkAttachmentReference basePassDepthAttachmentReference{};
        // basePassDepthAttachmentReference.attachment = 1;
        // basePassDepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // VkAttachmentReference basePassResolveAttachmentReference{};
        // basePassResolveAttachmentReference.attachment = VK_ATTACHMENT_UNUSED;
        // basePassResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        // VkSubpassDescription basePass = subpasses[MainCameraSubpassBasePass];
        // basePass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // // 明确指出这是一个图形子通道
        // basePass.colorAttachmentCount = 1;
        // basePass.pColorAttachments = &basePassColorAttachmentReference;
        // basePass.pDepthStencilAttachment = &basePassDepthAttachmentReference;
        // basePass.pResolveAttachments = &basePassResolveAttachmentReference;

        // VkAttachmentReference uiPassColorAttachmentReference {};
        // uiPassColorAttachmentReference.attachment = &backupEvenColorAttachmentDescription - attachments;
        // uiPassColorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // uint32_t uiPassPreserveAttachment = &backupOddColorAttachmentDescription - attachments;

        // VkAttachmentReference uiPassColorAttachmentReference {};
        // uiPassColorAttachmentReference.attachment = &swapchainImageAttachmentDescription - attachments;
        // uiPassColorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        // VkSubpassDescription uiPass     = subpasses[MainCameraSubpassUi];
        // uiPass.pipelineBindPoint        = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // uiPass.inputAttachmentCount     = 0;
        // uiPass.pInputAttachments        = NULL;
        // uiPass.colorAttachmentCount     = 1;
        // uiPass.pColorAttachments        = &uiPassColorAttachmentReference;
        // uiPass.pDepthStencilAttachment  = NULL;

        // uiPass.preserveAttachmentCount  = 1;
        // uiPass.pPreserveAttachments     = NULL;


        // VkSubpassDependency dependencies[MainCameraSubpassCount] = {};
        // VkSubpassDependency& basePassMainCameraSubpass = dependencies[0];
        // basePassMainCameraSubpass.srcSubpass           = VK_SUBPASS_EXTERNAL;
        // basePassMainCameraSubpass.dstSubpass           = MainCameraSubpassBasePass;
        // basePassMainCameraSubpass.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // basePassMainCameraSubpass.dstStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        // basePassMainCameraSubpass.srcAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        // basePassMainCameraSubpass.dstAccessMask        = VK_ACCESS_SHADER_READ_BIT;
        // basePassMainCameraSubpass.dependencyFlags      = 0;

        // VkSubpassDependency& uiPassDependOnBasePass = dependencies[1];
        // uiPassDependOnBasePass.srcSubpass           = MainCameraSubpassBasePass;
        // uiPassDependOnBasePass.dstSubpass           = MainCameraSubpassUi;
        // uiPassDependOnBasePass.srcStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // uiPassDependOnBasePass.dstStageMask         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // uiPassDependOnBasePass.srcAccessMask        = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        // uiPassDependOnBasePass.dstAccessMask        = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        // uiPassDependOnBasePass.dependencyFlags      = VK_DEPENDENCY_BY_REGION_BIT;


        // VkRenderPassCreateInfo renderPassInfo{};
        // renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        // renderPassInfo.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
        // renderPassInfo.pAttachments    = attachments;
        // renderPassInfo.subpassCount    = (sizeof(subpasses) / sizeof(subpasses[0]));;
        // renderPassInfo.pSubpasses      = subpasses;
        // renderPassInfo.dependencyCount = 0;
        // renderPassInfo.dependencyCount = (sizeof(dependencies) / sizeof(dependencies[0]));;
        // renderPassInfo.pDependencies   = dependencies;


        if (vkCreateRenderPass(device_, &renderPassInfo, nullptr, currRenderPass_) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create render pass!");
        }

        return true;
    }


    bool FrameBuffer::createFramebuffer() 
    {
        LOG_INFO("=== FrameBuffer:: start create Framebuffer! ===");
        currFbo_->attachments.clear();


        // if (!isOffscreen()) {
            currFbo_->attachments.push_back(vkCtx_.swapChainImageViews()[vkCtx_.imageIndex()]);
        // }
        // if (colorReady_) {
        //     auto *texColor = getAttachmentColor();
        //     currFbo_->attachments.push_back(texColor->createAttachmentView(VK_IMAGE_ASPECT_COLOR_BIT, colorAttachment_.layer, colorAttachment_.level));
        // }
        if (depthReady_) {
            auto *texDepth = getAttachmentDepth();
            currFbo_->attachments.push_back(texDepth->createAttachmentView(VK_IMAGE_ASPECT_DEPTH_BIT, depthAttachment_.layer, depthAttachment_.level));

            // currFbo_->attachments.push_back(texDepth->createAttachmentView(VK_IMAGE_ASPECT_DEPTH_BIT, depthAttachment_.layer, depthAttachment_.level));
            // currFbo_->attachments.push_back(texDepth->createAttachmentView(VK_IMAGE_ASPECT_DEPTH_BIT, depthAttachment_.layer, depthAttachment_.level));
        }

        // for (int i = 0; i < 2; i++) {
        //     TextureDesc desc = {};
        //     desc.width = vkCtx_.swapChainExtent().width;
        //     desc.height = vkCtx_.swapChainExtent().height;
        //     desc.type = TextureType_2D;
        //     desc.format = TextureFormat_RGBA_FLOAT16;
        //     desc.usage = TextureUsage_AttachmentColor;
        //     desc.useMipmaps = false;
        //     desc.multiSample = false;
        //     auto image = new TextureVulkan(vkCtx_, desc);

        //     auto attachment = image->createAttachmentView(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);

        //     currFbo_->attachments.push_back(attachment);
        // }


        // if (colorReady_ && isMultiSample()) {
        //     auto *texColor = getAttachmentColor();
        //     currFbo_->attachments.push_back(texColor->createResolveView());
        // }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *currRenderPass_;
        // 应绑定到渲染通道数组中相应附件描述的attachmentCount对象
        framebufferInfo.attachmentCount = currFbo_->attachments.size();
        framebufferInfo.pAttachments = currFbo_->attachments.data();
        // framebufferInfo.width = width_;
        // framebufferInfo.height = height_;
        // FIXME: 这里需要修改
        // framebufferInfo.attachmentCount = vkCtx_.swapChainImageViews().size();
        // framebufferInfo.pAttachments = vkCtx_.swapChainImageViews().data();
        framebufferInfo.width = vkCtx_.swapChainExtent().width;
        framebufferInfo.height = vkCtx_.swapChainExtent().height;
        framebufferInfo.layers = 1;
        VK_CHECK(vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &currFbo_->framebuffer));

        LOG_INFO("=== FrameBuffer:: end create Framebuffer! ===");
        return true;
    }


    void FrameBuffer::transitionLayoutBeginPass(VkCommandBuffer cmdBuffer) 
    {
        if (!isOffscreen()) 
        {
            return;
        }
        // if (isColorReady()) {
        //     auto *colorTex = getAttachmentColor();
        //     if (colorTex->usage & TextureUsage_Sampler) {
        //         VkImageSubresourceRange subRange{};
        //         subRange.aspectMask = colorTex->getImageAspect();
        //         subRange.baseMipLevel = getColorAttachment().level;
        //         subRange.baseArrayLayer = getColorAttachment().layer;
        //         subRange.levelCount = 1;
        //         subRange.layerCount = 1;

        //         TextureVulkan::transitionImageLayout(cmdBuffer, colorTex->getVkImage(), subRange,
        //                                             0,
        //                                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        //                                             VK_IMAGE_LAYOUT_UNDEFINED,
        //                                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        //                                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        //                                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        //     }
        // }
        if (isDepthReady()) {
            auto *depthTex = getAttachmentDepth();

            if (depthTex->usage & TextureUsage_Sampler) {
            VkImageSubresourceRange subRange{};
            subRange.aspectMask = depthTex->getImageAspect();
            subRange.baseMipLevel = getDepthAttachment().level;
            subRange.baseArrayLayer = getDepthAttachment().layer;
            subRange.levelCount = 1;
            subRange.layerCount = 1;

            TextureVulkan::transitionImageLayout(cmdBuffer, depthTex->getVkImage(), subRange,
                                                0,
                                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, // 用来存储图像，该图像用作深度或模板附件
                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,  // 图形管线任何一部分的操作都已经完成
                                                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT); // 在片段着色器开始运行之前可能发生的所有逐片段测试都已经完成了
            }
        }
    }


    void FrameBuffer::transitionLayoutEndPass(VkCommandBuffer cmdBuffer) 
    {
        if (!isOffscreen()) 
        {
            return;
        }
        // if (isColorReady()) {
        //     auto *colorTex = getAttachmentColor();
        //     if (colorTex->usage & TextureUsage_Sampler) {
        //         VkImageSubresourceRange subRange{};
        //         subRange.aspectMask = colorTex->getImageAspect();
        //         subRange.baseMipLevel = getColorAttachment().level;
        //         subRange.baseArrayLayer = getColorAttachment().layer;
        //         subRange.levelCount = 1;
        //         subRange.layerCount = 1;

        //         TextureVulkan::transitionImageLayout(cmdBuffer, colorTex->getVkImage(), subRange,
        //                                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        //                                             VK_ACCESS_SHADER_READ_BIT,
        //                                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        //                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        //                                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        //                                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        //     }
        // }
        if (isDepthReady()) 
        {
            auto *depthTex = getAttachmentDepth();

            if (depthTex->usage & TextureUsage_Sampler) 
            {
                VkImageSubresourceRange subRange{};
                subRange.aspectMask = depthTex->getImageAspect();
                subRange.baseMipLevel = getDepthAttachment().level;
                subRange.baseArrayLayer = getDepthAttachment().layer;
                subRange.levelCount = 1;
                subRange.layerCount = 1;

                TextureVulkan::transitionImageLayout(cmdBuffer, depthTex->getVkImage(), subRange,
                                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, // 用来存储图像，该图像用作深度或模板附件
                                                    VK_ACCESS_SHADER_READ_BIT, // 引用的内存用于存储一个图像对象，在着色器内使用图像加载或者纹理读取的操作来读取该对象
                                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, // 在片段着色器开始运行之后可能发生的所有逐片段测试都已经完成了
                                                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT); // 当一个绘制命令产生的所有片段着色器调用都完成时，这个阶段通过
            }
        }
    }   


    std::vector<VkSemaphore> &FrameBuffer::getAttachmentsSemaphoresWait() 
    {
    //     attachmentsSemaphoresWait_.clear();
    //     if (colorReady_) {
    //         auto waitSem = getAttachmentColor()->getSemaphoreWait();
    //         if (waitSem != VK_NULL_HANDLE) {
    //             attachmentsSemaphoresWait_.push_back(waitSem);
    //         }
    //     }

    //     if (depthReady_) {
    //         auto waitSem = getAttachmentDepth()->getSemaphoreWait();
    //         if (waitSem != VK_NULL_HANDLE) {
    //             attachmentsSemaphoresWait_.push_back(waitSem);
    //         }
    //     }

    //     return attachmentsSemaphoresWait_;
    }


    std::vector<VkSemaphore> &FrameBuffer::getAttachmentsSemaphoresSignal() 
    {
    //     attachmentsSemaphoresSignal_.clear();
    //     if (colorReady_) {
    //         auto signalSem = getAttachmentColor()->getSemaphoreSignal();
    //         if (signalSem != VK_NULL_HANDLE) {
    //             attachmentsSemaphoresSignal_.push_back(signalSem);
    //         }
    //     }

    //     if (depthReady_) {
    //         auto signalSem = getAttachmentDepth()->getSemaphoreSignal();
    //         if (signalSem != VK_NULL_HANDLE) {
    //             attachmentsSemaphoresSignal_.push_back(signalSem);
    //         }
    //     }

    //     return attachmentsSemaphoresSignal_;
    }

}

