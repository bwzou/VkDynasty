#pragma once

#include "runtime/render/RenderPass.h"
#include <vulkan/vulkan.h>
#include "runtime/render/passes/UIPass.h"
#include "runtime/render/passes/CombineUIPass.h"

#include<iostream>


namespace DynastyEngine 
{
    struct MainCameraPassInitInfo : RenderPassInitInfo
    {
        bool enableFxaa;
    };

    class MainCameraPass : public RenderPass
    {
    public:
        // 1: per mesh layout 
        // 2: global layout
        enum LayoutType : uint8_t
        {
            PerMesh = 0,
            // MeshGlobal,
            LayoutTypeCount 
        };

        // 1. model
        enum RenderPipeLineType : uint8_t
        {
            RenderPipelineTypeMeshGBuffer = 0,
            RenderPipelineTypeCount
        };

        void initialize(const RenderPassInitInfo* initInfo) override final;
        void draw(UIPass&           uiPass,
                  CombineUIPass&    combineUIPass,
                  uint32_t          currentSwapchainImageIndex);

        
        void setupAttachments();
        void setupRenderPass();
        void setupDescriptorSetLayout();
        void setupDescriptorSet();
        void setupPipelines(); 
        void setupSwapchainFramebuffers();
        void updateAfterFramebufferRecreate();
        
        void drawMeshGbuffer();

    public:
        std::vector<VkFramebuffer>  mSwapchainFramebuffers;
        
        
    };
}