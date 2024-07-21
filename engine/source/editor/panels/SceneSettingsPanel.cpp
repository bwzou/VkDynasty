#include "SceneSettingsPanel.h"

namespace DynastyEngine 
{
    SceneSettingsPanel::SceneSettingsPanel() {
        
    }

    void SceneSettingsPanel::onImGuiDraw(bool* pOpen) {
        if (!ImGui::Begin("Scene Settings", pOpen))
        {
            ImGui::End();
            return;
        }
        const char* modes[] = { "None", "Environment Hdr", "Sky Box" };
		ImGui::Text("Mode");
		ImGui::SameLine();

        ImGui::End();
    }
}