#pragma once

namespace DynastyEngine 
{
    class SceneSettingsPanel {
    public:
        SceneSettingsPanel() {
            
        }

        void onImGuiDraw(bool* pOpen);
    };
}