#pragma once

#include <stdexcept>
#include <array>
#include <memory>
#include <optional>


namespace DynastyEngine 
{
    class VulkanAPI;
    class DebugManager;
    class WindowSystem;
    class RenderCamera;
    class RenderPipeline;
    class RenderResource;
    class WindowUI;

    struct RenderSystemInitInfo
    {
        std::shared_ptr<WindowSystem> windowSystem;
        std::shared_ptr<DebugManager> debugManager;
    };

    class RenderSystem
    {
    public:
        RenderSystem() = default;
        ~RenderSystem();

        void initialize(RenderSystemInitInfo initInfo);
        void initializeUIRenderBackend(WindowUI* windowUI);
        void updateEngineContentViewport(float offsetX, float offsetY, float width, float height);
        void tick(float deltaTime);
        void clear();

        std::shared_ptr<RenderCamera> getRenderCamera();

    private:
        std::shared_ptr<VulkanAPI>                mVulkanAPI;
        std::shared_ptr<RenderCamera>             mRenderCamera;
        std::shared_ptr<RenderPipeline>           mRenderPipeline;
        std::shared_ptr<RenderResource>           mRenderResource;
    };
}