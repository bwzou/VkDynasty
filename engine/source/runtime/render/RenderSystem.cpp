#include "runtime/render/RenderSystem.h"
#include "runtime/platform/vulkan/VulkanAPI.h"
#include "runtime/render/RenderCamera.h"
#include "runtime/render/RenderPipeline.h"
#include "runtime/render/RenderResource.h"
#include "runtime/ui/WindowUI.h"


namespace DynastyEngine 
{
    // RenderSystem::RenderSystem() noexcept
    // {

    // }

    RenderSystem::~RenderSystem()
    {
        clear();
    }

    void RenderSystem::initialize(RenderSystemInitInfo initInfo)
    {
        LOG_INFO("RenderSystem::initialize")
        VulkanInitInfo vulkanInitInfo;
        vulkanInitInfo.windowSystem = initInfo.windowSystem;
        mVulkanAPI = std::make_shared<VulkanAPI>();
        mVulkanAPI->initialize(vulkanInitInfo);

        // setup render camera
        // camera
        mRenderCamera = std::make_shared<RenderCamera>();
        mRenderCamera->setPerspective(glm::radians(CAMERA_FOV), (float) SCR_WIDTH / (float) SCR_HEIGHT, CAMERA_NEAR, CAMERA_FAR);

        // setup render scene

        LOG_INFO("initialize render resource")
        // initialize render resource
        mRenderResource = std::make_shared<RenderResource>();
        mRenderResource->createVertexBuffer(mVulkanAPI);
        LOG_INFO("mRenderResource->createVertexBuffer")
        mRenderResource->createIndexBuffer(mVulkanAPI);
        LOG_INFO("mRenderResource->createIndexBuffer")
        mRenderResource->createUniformBuffer(mVulkanAPI);
        LOG_INFO("mRenderResource->createUniformBuffer")
        
        LOG_INFO("initialize render pipeline")
        // initialize render pipeline
        mRenderPipeline             = std::make_shared<RenderPipeline>();
        mRenderPipeline->mVulkanAPI = mVulkanAPI;
        mRenderPipeline->initialize(mVulkanAPI, mRenderResource);
    }

    void RenderSystem::initializeUIRenderBackend(WindowUI* windowUI)
    {
        mRenderPipeline->initializeUIRenderBackend(windowUI);
    }

    void RenderSystem::updateEngineContentViewport(float offsetX, float offsetY, float width, float height)
    {
        mVulkanAPI->mViewport.x        = offsetX;
        mVulkanAPI->mViewport.y        = offsetY;
        mVulkanAPI->mViewport.width    = width;
        mVulkanAPI->mViewport.height   = height;
        mVulkanAPI->mViewport.minDepth = 0.0f;
        mVulkanAPI->mViewport.maxDepth = 1.0f;
    }
    
    void RenderSystem::tick(float deltaTime)
    {
        // prepare render command context
        mVulkanAPI->prepareContext();

        // forward 前向渲染
        // if (mRenderPipelineType == RENDER_PIPELINE_TYPE::FORWARD_PIPELINE) {

        // }
        // mRenderPipeline->forwardRender();

        // deferred 延迟渲染
        mRenderPipeline->deferredRender();
        // throw std::runtime_error("退出");
    }
    
    void RenderSystem::clear()
    {
        if (mVulkanAPI) 
        {
            mVulkanAPI->clear();
        }
        mVulkanAPI.reset();

        if (mRenderResource) 
        {
            mRenderResource->clear();
        }
        mRenderResource.reset();

        if (mRenderPipeline)
        {
            mRenderPipeline->clear();
        }
        mRenderPipeline.reset();

    }
}