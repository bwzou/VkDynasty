#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <iostream>
#include "../function/Config.h"
#include "./panels/EngineSettingsPanel.h"
#include "./panels/SceneSettingsPanel.h"
#include "./panels/SceneHierarchyPanel.h"
#include "./panels/ContentBrowserPanel.h"
#include "./panels/ConfigPanel.h"


namespace DynastyEngine
{

    const uint32_t WIDTH = 1000;
    const uint32_t HEIGHT = 800;

    static void glfwErrorCallback(int error, const char *description) {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    class DynastyEditorUI
    {
    public:
        DynastyEditorUI(Config &config) : mConfig(config)
        {

        }

        ~DynastyEditorUI()
        {
            
        }

        GLFWwindow* getWindow() 
        {
            return mWindow;
        }

        std::shared_ptr<ConfigPanel> propertiesPanel() 
        {
            return mConfigPanel;
        }

        int initWindow();
        void onDraw();
        void cleanup();

    private:
        /* data */
        GLFWwindow* mWindow;
        Config &mConfig;
        SceneHierarchyPanel mSceneHierarchyPanel;
        ContentBrowserPanel mContentBrowserPanel;
        SceneSettingsPanel mSceneSettingsPanel;
        std::shared_ptr<ConfigPanel> mConfigPanel;
    };

}