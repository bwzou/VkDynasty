#include "runtime/render/RenderPass.h"
#include "runtime/platform/vulkan/VulkanAPI.h"
#include "runtime/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/code/util/FileUtils.h"
#include "runtime/ui/WindowUI.h"



namespace DynastyEngine
{

    void RenderPass::initialize(const RenderPassInitInfo* initInfo)
    {

    }

    void RenderPass::initializeUIRenderBackend(WindowUI* windowUI) {}
     
    void RenderPass::draw()
    {
        
    }

    void RenderPass::setCommonInfo(RenderPassCommonInfo commonInfo)
    {
        mVulkanAPI             = commonInfo.vulkanAPI;
        mRenderResource        = commonInfo.renderResource;
    }

    void RenderPass::compileGLSLFile(std::vector<VkShaderModule> &modules, std::vector<VkPipelineShaderStageCreateInfo> &stages, std::string source) 
    {
        std::string shaderGlslDir = gRuntimeGlobalContext.mConfigManager->getShaderGlslDir();
        std::string vsPath = shaderGlslDir + source + ".vert.spv";
        std::string fsPath = shaderGlslDir + source + ".frag.spv";

        const std::vector<char> vsCode = FileUtils::readFile(vsPath);
        const std::vector<char> fsCode = FileUtils::readFile(fsPath);

        mVulkanAPI->createShaderModule(modules[0], vsCode);
        mVulkanAPI->createShaderModule(modules[1], fsCode);

        mVulkanAPI->createShaderStages(modules, stages[0], stages[1]);
    }

    VkRenderPass RenderPass::getRenderPass() const
    {
        return mFramebuffer.renderPass;
    }

    std::vector<VkImageView> RenderPass::getFramebufferImageViews() const
    {
        std::vector<VkImageView> imageViews;
        for (auto &attach : mFramebuffer.attachments)
        {
            imageViews.push_back(attach.view);
        }
        return imageViews;
    }
    
    std::vector<VkDescriptorSetLayout> RenderPass::getDesciptorSetLatyouts() const
    {
        std::vector<VkDescriptorSetLayout> layouts;
        for (auto& desc : mDescriptorInfos) 
        {
            layouts.push_back(desc.layout);
        }
        return layouts;
    }
}
