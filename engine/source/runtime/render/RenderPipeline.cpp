#include "runtime/render/RenderPipeline.h"
#include "runtime/render/passes/UIPass.h"
#include "runtime/render/passes/CombineUIPass.h"
#include "runtime/render/passes/MainCameraPass.h"
#include "runtime/platform/vulkan/VulkanAPI.h"
#include "runtime/render/RenderResource.h"
#include "runtime/ui/WindowUI.h"



namespace DynastyEngine
{
    void RenderPipeline::initialize(std::shared_ptr<VulkanAPI> vulkanAPI, std::shared_ptr<RenderResource> renderResource)
    {
        mVulkanAPI          = vulkanAPI;
        mMainCameraPass     = std::make_shared<MainCameraPass>();
        mUIPass             = std::make_shared<UIPass>();
        mCombineUIPass      = std::make_shared<CombineUIPass>();

        RenderPassCommonInfo passCommonInfo;
        passCommonInfo.vulkanAPI = mVulkanAPI;
        passCommonInfo.renderResource = renderResource;

        mMainCameraPass->setCommonInfo(passCommonInfo);
        mUIPass->setCommonInfo(passCommonInfo);
        mCombineUIPass->setCommonInfo(passCommonInfo);

        MainCameraPassInitInfo mainCameraInfoInfo;
        mMainCameraPass->initialize(&mainCameraInfoInfo);

        UIPassInitInfo uiInitInfo;
        uiInitInfo.renderPass = mMainCameraPass->getRenderPass();
        mUIPass->initialize(&uiInitInfo);

        // CombineUIPassInitInfo combineUIInitInfo;
        // combineUIInitInfo.renderPass = mMainCameraPass->getRenderPass();
        // combineUIInitInfo.sceneInputAttachment = mMainCameraPass->getFramebufferImageViews()[MainCameraPassBackupBufferOdd];
        // combineUIInitInfo.uiInputAttachment = mMainCameraPass->getFramebufferImageViews()[MainCameraPassBackupBufferEven];
        // mCombineUIPass->initialize(&combineUIInitInfo);
    }   
        
    void RenderPipeline::forwardRender()
    {
        
    }
        
    void RenderPipeline::deferredRender()
    {
        LOG_INFO("waitForFences");
        mVulkanAPI->waitForFences();
        LOG_INFO("resetCommandPool");
        mVulkanAPI->resetCommandPool();
        LOG_INFO("prepareBeforePass");
        mVulkanAPI->prepareBeforePass();

        UIPass& uiPass                = *(static_cast<UIPass*>(mUIPass.get()));
        CombineUIPass& combineUIPass  = *(static_cast<CombineUIPass*>(mCombineUIPass.get()));
        static_cast<MainCameraPass*>(mMainCameraPass.get())->draw(uiPass, combineUIPass, mVulkanAPI->mCurrentSwapchainImageIndex);

        mVulkanAPI->submitAfterPass();
    }

    void RenderPipeline::initializeUIRenderBackend(WindowUI* windowUI)
    {
        mUIPass->initializeUIRenderBackend(windowUI);
    }
}