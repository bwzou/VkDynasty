#pragma once

#include "VulkanContext.h"
#include "RenderStates.h"
#include "VulkanUtils.h"
#include "RenderStates.h"
#include "TextureVulkan.h"
#include "runtime/code/base/UUID.h"
#include "runtime/code/base/macro.h"
#include "runtime/function/Texture.h"


namespace DynastyEngine
{

    enum
    {
        // _main_camera_pass_gbuffer_a                     = 0,
        // _main_camera_pass_gbuffer_b                     = 1,
        // _main_camera_pass_gbuffer_c                     = 2,
        // _main_camera_pass_backup_buffer_odd             = 3,
        // _main_camera_pass_backup_buffer_even            = 4,
        // _main_camera_pass_post_process_buffer_odd       = 5,
        // _main_camera_pass_post_process_buffer_even      = 6,
        MainCameraPassSwapchainImage = 0,
        MainCameraPassDepth,
        // MainCameraPassBackupBufferEven,
        // MainCameraPassBackupBufferOdd,
        // _main_camera_pass_custom_attachment_count       = 5,
        // _main_camera_pass_post_process_attachment_count = 2,
        MainCameraPassAttachmentCount
    };

    enum
    {
        MainCameraSubpassBasePass = 0,
        // _main_camera_subpass_deferred_lighting,
        // _main_camera_subpass_forward_lighting,
        // _main_camera_subpass_tone_mapping,
        // _main_camera_subpass_color_grading,
        // _main_camera_subpass_fxaa,
        // MainCameraSubpassUi,
        // _main_camera_subpass_combine_ui,
        MainCameraSubpassCount
    };

    struct FrameBufferAttachment 
    {
        std::shared_ptr<Texture> tex = nullptr;
        uint32_t layer = 0; // for cube map
        uint32_t level = 0;
    };


    struct FrameBufferContainer 
    {
        std::vector<VkImageView> attachments;
        VkFramebuffer framebuffer =  VK_NULL_HANDLE;
    };


    class FrameBuffer 
    {
    protected:
        bool offscreen_ = false;
        bool colorReady_ = false;
        bool depthReady_ = false;

        // TODO MTR
        FrameBufferAttachment colorAttachment_{};
        FrameBufferAttachment depthAttachment_{};    

    private:
        UUID<FrameBuffer> uuid_;
        VulkanContext& vkCtx_; 
        VkDevice device_ = VK_NULL_HANDLE;

        uint32_t width_ = 0;
        uint32_t height_ = 0;

        bool renderPassDirty_ = true;
        bool fboDirty_ = true;

        ClearStates clearStates_{};

        VkRenderPass *currRenderPass_ = nullptr;
        FrameBufferContainer *currFbo_ = nullptr;

        // TODO purge outdated elements
        std::vector<VkRenderPass> renderPassCache_;
        std::vector<FrameBufferContainer> fboCache_;

        std::vector<VkSemaphore> attachmentsSemaphoresWait_;
        std::vector<VkSemaphore> attachmentsSemaphoresSignal_;
        
        // 创建renderpass 
        bool createRenderPass();
        // 创建Framebuffer 
        bool createFramebuffer();
        

    public:
        FrameBuffer(VulkanContext &ctx, bool offscreen) : vkCtx_(ctx), offscreen_(offscreen) 
        {
            device_ = ctx.device();
        }
        ~FrameBuffer() 
        {

        }

        int getId() const 
        {
            return uuid_.get();
        }

        bool isValid() 
        {
            return colorReady_ || depthReady_;
        }

        void setColorAttachment(std::shared_ptr<Texture> &color, int level) 
        {
            if (color == colorAttachment_.tex && level == colorAttachment_.level) {
                return;
            }

            fboDirty_ = true;
            if (color != colorAttachment_.tex) {
                renderPassDirty_ = true;
            }

            colorAttachment_.tex = color;
            colorAttachment_.layer = 0;
            colorAttachment_.level = level;
            colorReady_ = true;
            width_ = color->getLevelWidth(level);
            height_ = color->getLevelHeight(level);
        }

        void setColorAttachment(std::shared_ptr<Texture> &color, CubeMapFace face, int level) 
        {
            if (color == colorAttachment_.tex && face == colorAttachment_.layer && level == colorAttachment_.level) 
            {
                return;
            }

            fboDirty_ = true;
            if (color != colorAttachment_.tex) 
            {
                renderPassDirty_ = true;
            }

            colorAttachment_.tex = color;
            colorAttachment_.layer = face;
            colorAttachment_.level = level;
            colorReady_ = true;
            width_ = color->getLevelWidth(level);
            height_ = color->getLevelHeight(level);
        }   

        void setDepthAttachment(std::shared_ptr<Texture> &depth) 
        {
            if (depth == depthAttachment_.tex) 
            {
                return;
            }

            fboDirty_ = true;
            renderPassDirty_ = true;

            depthAttachment_.tex = depth;
            depthAttachment_.layer = 0;
            depthAttachment_.level = 0;
            depthReady_ = true;
            width_ = depth->width;
            height_ = depth->height;
        }

        const FrameBufferAttachment &getColorAttachment() const 
        {
            return colorAttachment_;
        }

        const FrameBufferAttachment &getDepthAttachment() const 
        {
            return depthAttachment_;
        }

        void transitionLayoutBeginPass(VkCommandBuffer cmdBuffer);
        void transitionLayoutEndPass(VkCommandBuffer cmdBuffer);

        std::vector<VkSemaphore> &getAttachmentsSemaphoresWait();
        std::vector<VkSemaphore> &getAttachmentsSemaphoresSignal();

        bool create(const ClearStates &states) 
        {
            LOG_INFO( "=== FrameBuffer: start create FrameBuffer! ===");
            // TODO 暂时注释掉
            // if (!isValid()) {
            //     return false;
            // }
            clearStates_ = states;

            bool success = true;
            renderPassDirty_ = true;
            fboDirty_ = true;

            if (renderPassDirty_) 
            {
                renderPassDirty_ = false;
                renderPassCache_.emplace_back(); // emplace_back()则直接在容器尾部创建这个元素省去了拷贝或移动元素的过程
                currRenderPass_ = &renderPassCache_.back();
                success = createRenderPass();
            }
            if (success && fboDirty_) 
            { 
                fboDirty_ = false;
                fboCache_.emplace_back();
                currFbo_ = &fboCache_.back();
                success = createFramebuffer();
            }

            LOG_INFO( "=== FrameBuffer: end create FrameBuffer! ===");
            return success;
        }

        ClearStates &getClearStates() 
        {
            return clearStates_;
        }

        VkRenderPass &getRenderPass() 
        {
            return *currRenderPass_;
        }

        VkFramebuffer &getFrameBuffer() 
        {
            return currFbo_->framebuffer;
        }

        inline VkSampleCountFlagBits getSampleCount() 
        { 
            // if (colorReady_) {
            //     return getAttachmentColor()->getSampleCount();
            // }
            // if (depthReady_) {
            //     return getAttachmentDepth()->getSampleCount();
            // }

            return VK_SAMPLE_COUNT_1_BIT;
        }

        inline TextureVulkan *getAttachmentColor() 
        {
            if (colorReady_) {
                return dynamic_cast<TextureVulkan *>(colorAttachment_.tex.get());
            }
            return nullptr;
        }

        inline TextureVulkan *getAttachmentDepth() 
        {
            if (depthReady_) {
                return dynamic_cast<TextureVulkan *>(depthAttachment_.tex.get());
            }
            return nullptr;
        }

        uint32_t width() const 
        {
            return width_;
        }

        uint32_t height() const 
        {
            return height_;
        }

        bool isColorReady() const 
        {
            return colorReady_;
        }

        bool isDepthReady() const 
        {
            return depthReady_;
        }

        bool isMultiSample() const 
        {
            if (colorReady_) {
                return getColorAttachment().tex->multiSample;
            }
            if (depthReady_) {
                return getDepthAttachment().tex->multiSample;
            }

            return false;
        }

        bool isOffscreen() const 
        {
            return offscreen_;
        }

        void setOffscreen(bool offscreen) 
        {
            offscreen_ = offscreen;
        }

    };
}