#pragma once

#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>

namespace DynastyEngine 
{
    class SceneSettingsPanel {
    public:
        SceneSettingsPanel();

        void onImGuiDraw(bool* pOpen);

    private:
        // SkyBox
        // std::shared_ptr<Texture2D> mRight;
        // std::shared_ptr<Texture2D> mLeft;
        // std::shared_ptr<Texture2D> mTop;
        // std::shared_ptr<Texture2D> mBottom;
        // std::shared_ptr<Texture2D> mFront;
        // std::shared_ptr<Texture2D> mBack;
    };
}