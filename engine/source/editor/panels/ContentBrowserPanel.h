#pragma once

#include <imgui.h>
#include <imgui_internal.h>

namespace DynastyEngine 
{
    class ContentBrowserPanel {
    public:
        ContentBrowserPanel();

        void onImGuiDraw(bool* pOpen);
    };
}