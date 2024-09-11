#pragma once

#include "runtime/render/RenderPass.h"

namespace DynastyEngine
{
    struct CombineUIPassInitInfo : RenderPassInitInfo
    {
        VkRenderPass renderPass;
        VkImageView  sceneInputAttachment;
        VkImageView  uiInputAttachment;
    };

    class CombineUIPass : public RenderPass
    {
    public:
        void initialize(const RenderPassInitInfo* initInfo) override final;
        void draw();
        void updateAfterFramebufferRecreate(VkImageView sceneInputAttachment, VkImageView uiInputAttachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };
}