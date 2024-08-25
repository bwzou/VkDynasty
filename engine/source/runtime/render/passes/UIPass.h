#pragma once
#include "runtime/render/RenderPass.h"

namespace DynastyEngine
{
    class WindowUI;

    struct UIPassInitInfo: RenderPassInitInfo
    {
        VkRenderPass renderPass;
    };

    class UIPass : public RenderPass
    {
    public:
        void initialize(const RenderPassInitInfo* intiInfo) override final;
        void initializeUIRenderBackend(WindowUI* windowUI);
        void draw() override final;

    public:
        void uploadFonts();

    private:
        WindowUI*       mWindowUI;
    };
}