#include "runtime/render/RenderSystem.h"
#include "runtime/platform/vulkan/VulkanAPI.h"
#include "runtime/render/RenderCamera.h"
#include "runtime/render/RenderPipeline.h"
#include "runtime/render/RenderResource.h"
#include "runtime/ui/WindowUI.h"
#include "runtime/resource/config/CameraConfig.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/resource/config/GlobalRendering.h"
#include "runtime/resource/asset/AssetManager.h"



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

        std::shared_ptr<ConfigManager> configManager = gRuntimeGlobalContext.mConfigManager;
        ASSERT(configManager);
        std::shared_ptr<AssetManager> assetManager = gRuntimeGlobalContext.mAssetManager;
        ASSERT(assetManager);

        // global rendering resource
        GlobalRenderingRes globalRenderingRes;
        const std::string& globalRenderingResUrl = configManager->getGlobalRenderingResUrl();
        assetManager->loadAsset(globalRenderingResUrl, globalRenderingRes);

        // setup render camera
        // camera
        const CameraPose& cameraPose = globalRenderingRes.mCameraConfig.mPose;
        mRenderCamera = std::make_shared<RenderCamera>();
        LOG_INFO("cameraPose.mPosition {} {} {}", cameraPose.mPosition.x, cameraPose.mPosition.y, cameraPose.mPosition.z);
        mRenderCamera->lookAt(cameraPose.mPosition, cameraPose.mTarget, cameraPose.mUp);
        mRenderCamera->mZfar = globalRenderingRes.mCameraConfig.mZfar;
        mRenderCamera->mZnear = globalRenderingRes.mCameraConfig.mZnear;
        mRenderCamera->setAspect(globalRenderingRes.mCameraConfig.mAspect.x / globalRenderingRes.mCameraConfig.mAspect.y);

        // setup render scene

        LOG_INFO("initialize render resource")
        // initialize render resource
        mRenderResource = std::make_shared<RenderResource>();
        mRenderResource->createVertexBuffer(mVulkanAPI);
        LOG_INFO("mRenderResource->createVertexBuffer");
        mRenderResource->createIndexBuffer(mVulkanAPI);
        LOG_INFO("mRenderResource->createIndexBuffer");
        mRenderResource->createUniformBuffer(mVulkanAPI);
        LOG_INFO("mRenderResource->createUniformBuffer");
        
        LOG_INFO("initialize render pipeline");
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

        mRenderResource->updatePerFrameBuffer(mVulkanAPI, 0, mRenderCamera);

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

    std::shared_ptr<RenderCamera> RenderSystem::getRenderCamera() { return mRenderCamera; }
}