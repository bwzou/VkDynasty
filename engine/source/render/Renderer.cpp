#include "Renderer.h"


bool Renderer::create(GLFWwindow* window) {
    bool success = false;
#ifdef DEBUG
    success = vkCtx_.create(window, true);
#else
    success = vkCtx_.create(window, false);
#endif
    if (success) {
        device_ = vkCtx_.device();
    }
    return success;
}


bool Renderer::destroy() {

}


// framebuffer
std::shared_ptr<FrameBuffer> Renderer::createFrameBuffer(bool offscreen) {
    return std::make_shared<FrameBuffer>(vkCtx_, offscreen);
}


// texture
std::shared_ptr<Texture> Renderer::createTexture(const TextureDesc &desc) {

}


// vertex
std::shared_ptr<VertexArrayObject> Renderer::createVertexArrayObject(const VertexArray &vertexArray) {
    return std::make_shared<VertexArrayObjectVulkan>(vkCtx_, vertexArray);
}   


// shader program
std::shared_ptr<ShaderProgram> Renderer::createShaderProgram() {
    return std::make_shared<ShaderProgram>(vkCtx_); 
}


// pipeline states
std::shared_ptr<PipelineStates> Renderer::createPipelineStates(const RenderStates &renderStates) {
    return std::make_shared<PipelineStates>(vkCtx_, renderStates);
}


// uniform
std::shared_ptr<UniformBlock> Renderer::createUniformBlock(const std::string &name, int size) {
    return std::make_shared<UniformBlockVulkan> (vkCtx_, name, size);
}


std::shared_ptr<UniformSampler> createUniformSampler(const std::string &name, const TextureDesc &desc) {

}


void Renderer::beginRender() {
    // if (vkCtx_ == nullptr) {
    //     return;
    // }
    vkCtx_.beforeDrawFrame();
}


void Renderer::endRender() {
    // if (vkCtx_ == nullptr) {
    //     return;
    // }
    vkCtx_.afterDrawFrame(drawCmd_);
}


// pipeline
void Renderer::beginRenderPass(std::shared_ptr<FrameBuffer> &frameBuffer, const ClearStates &states) {
    std::cout << "----- beginRenderPass ----" << std::endl;
    if (!frameBuffer) {
        return;
    }

    commandBuffer_ = vkCtx_.beginCommands();

    drawCmd_ = commandBuffer_->cmdBuffer;

    fbo_ = dynamic_cast<FrameBuffer *>(frameBuffer.get());
    fbo_->create(states);

    clearValues_.clear();
    if (fbo_->isColorReady()) {
        VkClearValue colorClear;
        colorClear.color = {states.clearColor.r, states.clearColor.g, states.clearColor.b, states.clearColor.a};
        clearValues_.push_back(colorClear);
    }
    if (fbo_->isDepthReady()) {
        VkClearValue depthClear;
        depthClear.depthStencil = {states.clearDepth, 0};
        clearValues_.push_back(depthClear);
    }

    // transition attachments layout
    fbo_->transitionLayoutBeginPass(drawCmd_);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = fbo_->getRenderPass();
    // 我们为每个交换链图像创建了一个帧缓冲区，它被指定为颜色附件。
    renderPassInfo.framebuffer = fbo_->getFrameBuffer();
    // 定义渲染区域的大小。渲染区域定义着色器加载和存储将发生的位置
    renderPassInfo.renderArea.offset = {0, 0};
    // renderPassInfo.renderArea.extent = {fbo_->width(), fbo_->height()};
    // renderPassInfo.clearValueCount = clearValues_.size();
    // renderPassInfo.pClearValues = clearValues_.data();
    renderPassInfo.renderArea.extent = vkCtx_.swapChainExtent();
    // 透明颜色定义为简单的黑色，不透明度为 100%。
    VkClearValue clearColor = {{{0.0f, 0.5f, 0.5f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(drawCmd_, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}
    

void Renderer::setViewPort(int x, int y, int width, int height) {
    viewport_.x = (float) x;
    viewport_.y = (float) y;
    // 交换链及其图像的大小可能与窗口的WIDTH和HEIGHT不同。
    // 交换链的图像以后将被用作帧缓冲器，所以我们应该坚持它们的尺寸。
    viewport_.width = (float) width;
    viewport_.height = (float) height;
    // minDepth和maxDepth值指定了用于帧缓冲区的深度值范围。这些值必须在[0.0f, 1.0f]范围内
    viewport_.minDepth = 0.0f;
    viewport_.maxDepth = 1.0f;

    scissor_.extent.width = width;
    scissor_.extent.height = height;

    // viewport_.absMinDepth = std::min(viewport_.minDepth, viewport_.maxDepth);
    // viewport_.absMaxDepth = std::max(viewport_.minDepth, viewport_.maxDepth);

    // viewport_.innerO.x = viewport_.x + viewport_.width / 2.f;
    // viewport_.innerO.y = viewport_.y + viewport_.height / 2.f;
    // viewport_.innerO.z = viewport_.minDepth;
    // viewport_.innerO.w = 0.f;

    // viewport_.innerP.x = viewport_.width / 2.f;
    // viewport_.innerP.y = viewport_.height / 2.f;
    // viewport_.innerP.z = viewport_.maxDepth - viewport_.minDepth;
    // viewport_.innerP.w = 1.f;
}


void Renderer::setVertexArrayObject(std::shared_ptr<VertexArrayObject> &vao) {
    std::cout << "------------ setVertexArrayObject -----------" << std::endl;
    if (!vao) {
        return;
    }

    vao_ = dynamic_cast<VertexArrayObjectVulkan *>(vao.get());
}


void Renderer::setShaderProgram(std::shared_ptr<ShaderProgram> &program) {
    shaderProgram_ = dynamic_cast<ShaderProgram *>(program.get());
}


void Renderer::setShaderResources(std::shared_ptr<ShaderResources> &resources) {
    if (!resources) {
        return;
    }
    if (shaderProgram_) {
        shaderProgram_->beginBindUniforms(commandBuffer_);
        shaderProgram_->bindResources(*resources);
        shaderProgram_->endBindUniforms();
    }
}


void Renderer::setPipelineStates(std::shared_ptr<PipelineStates> &states) {
    std::cout << "------- setPipelineStates: state start! -----" << &states << std::endl;
    if (!states) {
        return;
    }
    std::cout << "------- setPipelineStates: state exist! -----" << &states << std::endl;
    pipelineStates_ = dynamic_cast<PipelineStates *>(states.get());
    pipelineStates_->create(vao_->getVertexInputInfo(), shaderProgram_, fbo_->getRenderPass(), fbo_->getSampleCount());
}


void Renderer::draw() {
    std::cout << "------- Renderer: draw start! -----" << drawCmd_ << std::endl;
    // 我们现在可以绑定图形管道：
    vkCmdBindPipeline(drawCmd_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineStates_->getGraphicsPipeline());

    // 视口基本上描述了输出将被渲染到的framebuffer的区域。
    vkCmdSetViewport(drawCmd_, 0, 1, &viewport_);
    // 只想在整个帧缓冲区内作画，所以我们将指定一个完全覆盖它的剪切矩形
    vkCmdSetScissor(drawCmd_, 0, 1, &scissor_);

    VkBuffer vertexBuffers[] = {vao_->getVertexBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(drawCmd_, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(drawCmd_, vao_->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

    // 实际将每个帧的正确描述符集绑定到着色器中的描述符vkCmdBindDescriptorSets。这需要在调用之前完成vkCmdDrawIndexed
    auto &descriptorSets = shaderProgram_->getDescriptorSet();
    vkCmdBindDescriptorSets(drawCmd_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineStates_->getGraphicsPipelineLayout(), 
                            0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

    vkCmdDrawIndexed(drawCmd_, vao_->getIndicesCnt(), 1, 0, 0, 0);
    std::cout << "------- Renderer: draw end! -----" << std::endl;
}


void Renderer::endRenderPass() {
    vkCmdEndRenderPass(drawCmd_);

    // transition attachments  layout
    fbo_->transitionLayoutEndPass(drawCmd_);
    semaphoresWait_.clear();
    semaphoresSignal_.clear();

    // semaphores from attachments
    // semaphoresWait_ = fbo_->getAttachmentsSemaphoresWait();
    // semaphoresSignal_ = fbo_->getAttachmentsSemaphoresSignal();
    
    // semaphores from last pass
    if (lastPassSemaphore_ != VK_NULL_HANDLE) {
        semaphoresWait_.push_back(lastPassSemaphore_);
    }
    semaphoresWait_.push_back(vkCtx_.imageAvailableSemaphore());
    if (commandBuffer_->semaphore != VK_NULL_HANDLE) {
        semaphoresSignal_.push_back(commandBuffer_->semaphore);
    }
    semaphoresSignal_.push_back(vkCtx_.renderFinishedSemaphore());
    std::cout << "semaphoresWait_" << semaphoresWait_.size() << std::endl;
    std::cout << "semaphoresSignal_" << semaphoresSignal_.size() << std::endl;

    vkCtx_.endCommands(commandBuffer_, semaphoresWait_, semaphoresSignal_);
    lastPassSemaphore_ = commandBuffer_->semaphore;
}


void Renderer::waitIdle() {
    if (vkQueueWaitIdle(vkCtx_.graphicsQueue()) != VK_SUCCESS) {
        std::runtime_error("vkQueueWaitIdle error");
    }
}