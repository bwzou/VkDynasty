#pragma once

#include<iostream>


namespace DynastyEngine
{
    class VulkanAPI;
    class RenderPass;
    class RenderResource;
    class WindowUI;
    
    class RenderPipeline
    {
    public:
        void initialize(std::shared_ptr<VulkanAPI> vulkanAPI, std::shared_ptr<RenderResource> renderResource);
        void initializeUIRenderBackend(WindowUI* window_ui);
        void forwardRender();
        void deferredRender();

        void passUpdateAfterRecreateSwapchain();
        void clear();

    public:
        std::shared_ptr<VulkanAPI>          mVulkanAPI;
        std::shared_ptr<RenderPass>         mUIPass;
        std::shared_ptr<RenderPass>         mCombineUIPass;
        std::shared_ptr<RenderPass>         mMainCameraPass;
    };
}