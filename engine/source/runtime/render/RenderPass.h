#pragma once

#include "runtime/render/RenderPipeline.h"

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>


namespace DynastyEngine
{

    enum
    {
        MainCameraPassGbufferA                     = 0,
        // MainCameraPassGbufferB                     = 1,
        // MainCameraPassGbufferC                     = 2,
        MainCameraPassBackupBufferOdd              = 1,
        // MainCameraPassBackupBufferEven             = 2,
        // _main_camera_pass_backup_buffer_odd             = 3,
        // _main_camera_pass_backup_buffer_even            = 4,
        // _main_camera_pass_post_process_buffer_odd       = 5,
        // _main_camera_pass_post_process_buffer_even      = 6,
        MainCameraPassDepth,
        MainCameraPassSwapchainImage,
        MainCameraPassCustomAttachmentCount        = 2,
        // _main_camera_pass_post_process_attachment_count = 2,
        // MainCameraPassAttachmentCount              = 7
        MainCameraPassAttachmentCount              = 4
    };

    enum
    {
        MainCameraSubpassBasePass = 0,
        // _main_camera_subpass_deferred_lighting,
        // _main_camera_subpass_forward_lighting,
        // _main_camera_subpass_tone_mapping,
        // _main_camera_subpass_color_grading,
        // _main_camera_subpass_fxaa,
        MainCameraSubpassUi,
        MainCameraSubpassCombineUI,
        MainCameraSubpassCount
    };
    
    class VulkanAPI;
    class WindowUI;
    class RenderResource;
    // class Renderer;

    // struct FramebufferAttachment
    // {
    //     /* data */
    //     VkImage*        image;
    //     VkDeviceMemory* mem;
    //     VkImageView*    view;
    //     VkFormat        format;
    // };
    
    // struct Framebuffer
    // {
    //     int                                width;
    //     int                                height;
    //     VkFramebuffer*                     framebuffer;
    //     VkRenderPass*                      renderPass;
    //     std::vector<FramebufferAttachment> attachments;
    // };
    
    struct RenderPassInitInfo
    {

    };

    struct RenderPassCommonInfo
    {
        std::shared_ptr<VulkanAPI>      vulkanAPI;
        std::shared_ptr<RenderResource> renderResource;
    };

    
    class RenderPass
    {
    public:
        virtual void initialize(const RenderPassInitInfo* initInfo);
        virtual void initializeUIRenderBackend(WindowUI* windowUI);
        virtual void draw();
        virtual void setCommonInfo(RenderPassCommonInfo commonInfo);
        
        void                                compileGLSLFile(std::vector<VkShaderModule> &modules, std::vector<VkPipelineShaderStageCreateInfo> &stages, const std::string source);
        VkRenderPass                        getRenderPass() const;
        std::vector<VkImageView>            getFramebufferImageViews() const;
        std::vector<VkDescriptorSetLayout>  getDesciptorSetLatyouts() const;

    protected:
        struct FramebufferAttachment
        {
            /* data */
            VkImage         image;
            VkDeviceMemory  mem;
            VkImageView     view;
            VkFormat        format;
        };
        
        struct Framebuffer
        {
            int                                width;
            int                                height;
            VkFramebuffer                      framebuffer;
            VkRenderPass                       renderPass;
            std::vector<FramebufferAttachment> attachments;
        };

        struct Descriptor
        {
            VkDescriptorSetLayout layout;
            VkDescriptorSet       descriptorSet;
        };

        struct RenderPipelineBase
        {
            VkPipelineLayout layout;
            VkPipeline       pipeline;
        };

        std::shared_ptr<VulkanAPI>                mVulkanAPI;
        Framebuffer                               mFramebuffer;
        std::vector<Descriptor>                   mDescriptorInfos;
        std::vector<RenderPipelineBase>           mRenderPipelines;
        std::shared_ptr<RenderResource>           mRenderResource;
    };
}