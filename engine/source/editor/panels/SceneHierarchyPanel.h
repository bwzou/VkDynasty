#pragma once

namespace DynastyEngine 
{
    class SceneHierarchyPanel {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const int temp);
        
        void setContent();

        void onImGuiDraw(bool* pOpen, bool* pOpenProperties);

        void getSelectedEntity() const;

        void setSelectedEntity();

        // void drawComponents(Entity entity);

    private:
        int mTemp;
    };
}