#pragma once

#include <iostream>
// #include <GLFW/glfw3.h>
#include "runtime/code/base/macro.h"


namespace DynastyEngine 
{
    class ImGuiLayer 
    {
    public:
        ImGuiLayer();
        // ImGuiLayer(GLFWwindow* window);
        ImGuiLayer(const std::string& name);
        ~ImGuiLayer();

        virtual void onAttach() {};
		virtual void onDetach() {};
        
        // virtual void OnEvent(Event& e); 

        virtual void begin() {};
        virtual void end() {};
        virtual void onImGuiDraw() {};

        static ImGuiLayer* create();

        void blockEvents(bool block) { mBlockEvents = block; }

        void setDarkThemeColors();

        [[nodiscard]] inline const std::string& GetName() const { return mDebugName; }

    // public:
    //     GLFWwindow* mWindow;
        
    private:
        bool mBlockEvents = true; 
        std::string mDebugName;  
    };
}
