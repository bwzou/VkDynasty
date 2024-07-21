#pragma once

namespace DynastyEngine 
{
    class ContentBrowserPanel {
    public:
        ContentBrowserPanel();

        void onImGuiDraw(bool* pOpen);
    };
}