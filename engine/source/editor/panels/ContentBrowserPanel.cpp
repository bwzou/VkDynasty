#include "ContentBrowserPanel.h"


namespace DynastyEngine 
{
    ContentBrowserPanel::ContentBrowserPanel() {
        
    }

    void ContentBrowserPanel::onImGuiDraw(bool* pOpen) {
        if (!ImGui::Begin("Content Browser", pOpen))
		{
			ImGui::End();
			return;
		}

        ImGui::Columns(2);

		static bool init = true;
		if (init)
		{
			ImGui::SetColumnWidth(0, 240.0f);
			init = false;
		}

        if (ImGui::BeginChild("CONTENT_BROWSER_TREE"))
		{
			// DrawTree();
		}
		ImGui::EndChild();

		ImGui::NextColumn();

		if (ImGui::BeginChild("CONTENT_BROWSER_CONTENT"))
		{
			// DrawContent();
		}
		ImGui::EndChild();

		ImGui::Columns(1);

		ImGui::End();
    }
}