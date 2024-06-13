#include <vulkan/vulkan.h>
#include "FrameBuffer.h"


bool FrameBuffer::createRenderPass() {
    // attachment参数通过附件描述数组中的索引指定要引用的附件
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = VK_ATTACHMENT_UNUSED;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = VK_ATTACHMENT_UNUSED;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference resolveAttachmentRef{};
    resolveAttachmentRef.attachment = VK_ATTACHMENT_UNUSED;
    resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::vector<VkAttachmentDescription> attachments;
    if (true) {
        // auto *colorTex = getAttachmentColor();

        // 只有一个单一的颜色缓冲区附件，由交换链中的一个图像代表
        VkAttachmentDescription colorAttachment{};
        // 颜色附件的format应该与交换链图像的格式相匹配
        colorAttachment.format = vkCtx_.swapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        
        // 这里采用存储操作。
        // stencilLoadOp / stencilStoreOp 适用于模板数据
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        colorAttachmentRef.attachment = attachments.size();
        attachments.push_back(colorAttachment);
    }

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

    if (depthReady_) {
        auto *depthTex = getAttachmentDepth();

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = cvtImageFormat(depthTex->format, depthTex->usage);
        depthAttachment.samples = getAttachmentDepth()->getSampleCount();
        depthAttachment.loadOp = clearStates_.depthFlag ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp = colorReady_ ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        depthAttachmentRef.attachment = attachments.size();
        attachments.push_back(depthAttachment);
    }

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
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // 明确指出这是一个图形子通道
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &resolveAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    // renderPassInfo.dependencyCount = 1;
    // renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device_, &renderPassInfo, nullptr, currRenderPass_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    return true;
}


bool FrameBuffer::createFramebuffer() {
    std::cout << "------- createFramebuffer: start! -----" << std::endl;
    currFbo_->attachments.clear();


    if (true) {
        currFbo_->attachments.push_back(vkCtx_.swapChainImageViews()[vkCtx_.imageIndex()]);
    }
    // if (colorReady_) {
    //     auto *texColor = getAttachmentColor();
    //     currFbo_->attachments.push_back(texColor->createAttachmentView(VK_IMAGE_ASPECT_COLOR_BIT, colorAttachment_.layer, colorAttachment_.level));
    // }
    if (depthReady_) {
        auto *texDepth = getAttachmentDepth();
        currFbo_->attachments.push_back(texDepth->createAttachmentView(VK_IMAGE_ASPECT_DEPTH_BIT, depthAttachment_.layer, depthAttachment_.level));
    }
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

    std::cout << "------- createFramebuffer: end! -----" << std::endl;
    return true;
}


void FrameBuffer::transitionLayoutBeginPass(VkCommandBuffer cmdBuffer) {
    // if (!isOffscreen()) {
    //     return;
    // }

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

    // if (isDepthReady()) {
    //     auto *depthTex = getAttachmentDepth();

    //     if (depthTex->usage & TextureUsage_Sampler) {
    //     VkImageSubresourceRange subRange{};
    //     subRange.aspectMask = depthTex->getImageAspect();
    //     subRange.baseMipLevel = getDepthAttachment().level;
    //     subRange.baseArrayLayer = getDepthAttachment().layer;
    //     subRange.levelCount = 1;
    //     subRange.layerCount = 1;

    //     TextureVulkan::transitionImageLayout(cmdBuffer, depthTex->getVkImage(), subRange,
    //                                         0,
    //                                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //                                         VK_IMAGE_LAYOUT_UNDEFINED,
    //                                         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    //                                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    //                                         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    //     }
    // }
}


void FrameBuffer::transitionLayoutEndPass(VkCommandBuffer cmdBuffer) {
    // if (!isOffscreen()) {
    //     return;
    // }
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

    // if (isDepthReady()) {
    //     auto *depthTex = getAttachmentDepth();

    //     if (depthTex->usage & TextureUsage_Sampler) {
    //         VkImageSubresourceRange subRange{};
    //         subRange.aspectMask = depthTex->getImageAspect();
    //         subRange.baseMipLevel = getDepthAttachment().level;
    //         subRange.baseArrayLayer = getDepthAttachment().layer;
    //         subRange.levelCount = 1;
    //         subRange.layerCount = 1;

    //         TextureVulkan::transitionImageLayout(cmdBuffer, depthTex->getVkImage(), subRange,
    //                                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //                                             VK_ACCESS_SHADER_READ_BIT,
    //                                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    //                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //                                             VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //                                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    //     }
    // }
}   


std::vector<VkSemaphore> &FrameBuffer::getAttachmentsSemaphoresWait() {
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


std::vector<VkSemaphore> &FrameBuffer::getAttachmentsSemaphoresSignal() {
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


