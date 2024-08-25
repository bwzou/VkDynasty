#pragma once

#include <memory>

namespace DynastyEngine
{
    class WindowSystem;
    class RenderSystem;
    struct WindowUIInitInfo
    {
        std::shared_ptr<WindowSystem> windowSystem;
        std::shared_ptr<RenderSystem> renderSystem;
    };

    class WindowUI
    {
    public:
        virtual void initialize(WindowUIInitInfo initInfo);
        virtual void renderUI();
    };
}