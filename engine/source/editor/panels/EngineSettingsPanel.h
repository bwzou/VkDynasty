#pragma once

namespace DynastyEngine 
{
    class EngineSettingsPanel {
    public:
        EngineSettingsPanel() = default;
        EngineSettingsPanel(const int temp);

        void onImGuiDraw(bool* pOpen);
    };
}