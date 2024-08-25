#pragma once
#include "runtime/framework/level/Level.h"


namespace DynastyEngine 
{
    class SceneHierarchyPanel {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const int temp);
        
        void setContent(const std::shared_ptr<Level>& context);

        void onImGuiDraw(bool* pOpen, bool* pOpenProperties);

        void getSelectedEntity() const;

        void setSelectedEntity();
        

        // void drawComponents(Entity entity);

    private:
        int mTemp;
    };
}