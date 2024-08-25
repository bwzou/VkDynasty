#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include "runtime/platform/vulkan/VulkanAPI.h"
#include "runtime/render/passes/UIPass.h"
#include "runtime/ui/WindowUI.h"
#include "runtime/render/RenderSystem.h"


namespace DynastyEngine
{
    void UIPass::initialize(const RenderPassInitInfo* intiInfo) 
    {
        RenderPass::initialize(nullptr);
        mFramebuffer.renderPass = static_cast<const UIPassInitInfo*>(intiInfo)->renderPass;
    }
    
    void UIPass::initializeUIRenderBackend(WindowUI* windowUI)
    {
        mWindowUI = windowUI;
        ImGui_ImplGlfw_InitForVulkan(mVulkanAPI->mWindow, true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance                  = mVulkanAPI->mInstance;
        initInfo.PhysicalDevice            = mVulkanAPI->mPhysicalDevice;
        initInfo.Device                    = mVulkanAPI->mDevice;
        initInfo.QueueFamily               = mVulkanAPI->mQueueIndices.graphicsFamily.value();
        initInfo.Queue                     = mVulkanAPI->mGraphicsQueue;
        initInfo.DescriptorPool            = mVulkanAPI->mDescriptorPool;
        initInfo.Subpass                   = 0;
        // may be different from the real swapchain image count
        // see ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
        initInfo.MinImageCount = 3;
        initInfo.ImageCount    = 3;
        // iniInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        // iniInfo.Allocator = VK_NULL_HANDLE;
        // iniInfo.CheckVkResultFn = checkVkResult;
        ImGui_ImplVulkan_Init(&initInfo, mFramebuffer.renderPass);

        uploadFonts();
    }

    void UIPass::uploadFonts()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool                 = mVulkanAPI->mCommandPool;
        allocInfo.commandBufferCount          = 1;

        VkCommandBuffer commandBuffer;
        if (!mVulkanAPI->allocateCommandBuffers(&allocInfo, commandBuffer))
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (!mVulkanAPI->beginCommandBuffer(commandBuffer, &beginInfo))
        {
            throw std::runtime_error("Could not create one-time command buffer!");
        }

        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

        if (!mVulkanAPI->endCommandBuffer(commandBuffer))
        {
            throw std::runtime_error("failed to record command buffer!");
        }

        std::vector<VkSubmitInfo> submitInfo {1};
        submitInfo[0].sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo[0].commandBufferCount = 1;
        submitInfo[0].pCommandBuffers    = &commandBuffer;

        mVulkanAPI->queueSubmit(mVulkanAPI->getGraphicsQueue(), 1, submitInfo, VK_NULL_HANDLE);
        mVulkanAPI->queueWaitIdle(mVulkanAPI->getGraphicsQueue());
        mVulkanAPI->freeCommandBuffers(mVulkanAPI->getCommandPoor(), 1, commandBuffer);

        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
    
    void UIPass::draw() 
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        mWindowUI->renderUI();

        ImGui::Render();
        ImDrawData* mainDrawData = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(mainDrawData, mVulkanAPI->mCurrentCommandBuffer);
    }
}