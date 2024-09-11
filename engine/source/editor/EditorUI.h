#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <iostream>
#include "runtime/function/Config.h"
#include "editor/panels/EngineSettingsPanel.h"
#include "editor/panels/SceneSettingsPanel.h"
#include "editor/panels/SceneHierarchyPanel.h"
#include "editor/panels/ContentBrowserPanel.h"
#include "editor/panels/ConfigPanel.h"

#include "runtime/ui/WindowUI.h"


namespace DynastyEngine
{
    class Level;


    // const uint32_t WIDTH = 1000;
    // const uint32_t HEIGHT = 800;

    static void glfwErrorCallback(int error, const char *description) {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    class EditorUI : public WindowUI
    {
    public:
        EditorUI() {}

        // EditorUI(Config &config) : mConfig(config)
        // {
        // 
        // }

        ~EditorUI()
        {
            
        }

        void initialize(WindowUIInitInfo initInfo);
        void showEditorUI();
        void showEditorMenu(bool* pEditorMenu);
        void showContentBrowserWindow(bool* bShowContentBrowser);
        void showEngineSettingsWindow(bool* bShowEngineSettings);
        void showEditorViewportWindow(bool* bShowViewport);
        void showSceneSettingsWindow(bool* bShowSceneSettings);
        void showSceneHierarchyWindow(bool* bShowSceneHierachy, bool* bShowProperties);

        void setUIColorStyle();
        void uploadFonts();

        inline void setReloadModelFunc(const std::function<bool(const std::string &)> &func) 
        {
            mReloadModelFunc = func;
        }
        inline void setUpdateLightFunc(const std::function<void(glm::vec3 &, glm::vec3 &)> &func) 
        {
            mUpdateLightFunc = func;
        }
        bool isInitialize() 
        {
            return pInitializeImgui;
        }

        std::shared_ptr<ConfigPanel> propertiesPanel() 
        {
            return mConfigPanel;
        }

        void init();
        
        void renderUI();

        void newScene();
        void openScene();
        void openScene(const std::filesystem::path& path);
        void saveScene(const std::filesystem::path& format);
        void saveSceneAs();

        void uiToolbar();
        void loadDefaultEditorConfig();

        // bool loadConfig();
        // bool reloadModel(const std::string &name); 

        void cleanup();

    private:
        /* data */
        GLFWwindow* mWindow;
        // Config &mConfig;
        Renderer* mRenderer;
        VkRenderPass mImguiRenderPass;

        
        glm::vec2 mViewportBounds[2];
        bool mViewportFocused = false; 
        bool mViewportHovered = false;




        std::unordered_map<std::string, std::string> mModelPaths;
        std::unordered_map<std::string, std::string> mSkyboxPaths;
        
        std::vector<const char *> mModelNames;
        std::vector<const char *> mSkyboxNames;

        std::function<bool(const std::string &path)> mReloadModelFunc;
        std::function<void(glm::vec3 &position, glm::vec3 &color)> mUpdateLightFunc;

        bool pInitializeImgui = false;

        SceneHierarchyPanel mSceneHierarchyPanel;
        ContentBrowserPanel mContentBrowserPanel;
        SceneSettingsPanel mSceneSettingsPanel;

        std::shared_ptr<ConfigPanel> mConfigPanel;

        std::shared_ptr<Level> mEditorScene;
        std::filesystem::path mEditorScenePath;
    };

}