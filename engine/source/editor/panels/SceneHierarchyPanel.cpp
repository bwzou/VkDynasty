#include "SceneHierarchyPanel.h"

namespace DynastyEngine 
{
    SceneHierarchyPanel::SceneHierarchyPanel(const int temp)  
    {

    }
        
    void SceneHierarchyPanel::setContent(const std::shared_ptr<Level>& context) 
    {
        
    }

    void SceneHierarchyPanel::onImGuiDraw(bool* pOpen, bool* pOpenProperties) 
    {
        if (*pOpen)
		{
			ImGui::Begin("Scene Hierarchy", pOpen);

			// if (mContext)
			// {
			// 	mContext->mRegistry.each([&](auto entityID)
			// 		{
			// 			Entity entity = { entityID, mContext.get() };
			// 			DrawEntityNode(entity);
			// 		});

			// 	if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			// 		mSelectionContext = {};

			// 	// Right-click on blank space
			// 	if (ImGui::BeginPopupContextWindow(0, 1, false))
			// 	{
			// 		if (ImGui::MenuItem("Create Empty Entity"))
			// 			mContext->CreateEntity("Empty Entity");

			// 		if (ImGui::MenuItem("Create Point Light"))
			// 		{
			// 			auto entity = mContext->CreateEntity("Point Light");
			// 			entity.AddComponent<PointLightComponent>();
			// 			SetSelectedEntity(entity);
			// 		}

			// 		if (ImGui::MenuItem("Create Directional Light"))
			// 		{
			// 			auto entity = mContext->CreateEntity("Directional Light");
			// 			entity.AddComponent<DirectionalLightComponent>();
			// 			SetSelectedEntity(entity);
			// 		}

			// 		if (ImGui::MenuItem("Create Audio"))
			// 		{
			// 			auto entity = mContext->CreateEntity("Audio");
			// 			entity.AddComponent<SoundComponent>();
			// 			SetSelectedEntity(entity);
			// 		}

			// 		ImGui::EndPopup();
			// 	}
			// }

			ImGui::End();
		}

        if (*pOpenProperties)
		{
			ImGui::Begin("Properties", pOpenProperties);
			// if (mSelectionContext)
			// {
			// 	DrawComponents(mSelectionContext);
			// }
			ImGui::End();
		}
    }

    void SceneHierarchyPanel::getSelectedEntity() const 
    { 

    }

    void SceneHierarchyPanel::setSelectedEntity() 
    {
        
    }
}