#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "./code/base/GLMInc.h"
#include "json11.hpp"

#include <stdlib.h>
#include <iostream>
// #include <stdexcept>
// #include <algorithm>
// #include <vector>
// #include <fstream>
// #include <chrono>
// #include <cstdint>
// #include <cstdlib>
// #include <cstring>
// #include <array>
// #include <limits>
// #include <optional>
// #include <set>
#include <unordered_map>
#include "runtime/function/Model.h"
#include "runtime/function/Material.h"
#include "runtime/function/Config.h"
#include "runtime/function/Camera.h"
#include "runtime/function/OrbitController.h"
#include "runtime/function/ModelLoader.h"
#include "runtime/code/util/FileUtils.h"
#include "runtime/code/base/macro.h"

#include "editor/EditorUI.h"
#include "editor/panels/ConfigPanel.h"
#include "runtime/Viewer.h"
#include "runtime/render/FrameBuffer.h"
#include "runtime/imgui/ImGuiLayer.h"
// #include "./platform/vulkan/VulkanImGuiLayer.h"


#define RENDER_TYPE_NONE (-1)


namespace DynastyEngine
{
    const unsigned int SCR_WIDTH = 1000;
    const unsigned int SCR_HEIGHT = 800;

    class Engine {
    public:
        bool initEngine(); 
        bool initEvent(GLFWwindow* window); 
        bool run();
        void drawFrame();
        void setupConfigPanelActions();
        void cleanup();

        inline void waitRenderIdle() 
        {
            if (rendererType_ != RENDER_TYPE_NONE) 
            {
                viewer_->waitRenderIdle();
            }
        }
        inline void resetStates() 
        {
            waitRenderIdle();
            modelLoader_->resetAllModelStates();
            modelLoader_->getScene().resetStates();
        }
        inline void drawPanel() 
        {
            if (showConfigPanel_) {
                // configPanel_->onDraw();
                editorUI_->onDraw();
            }
        }
        inline void togglePanelState() 
        {
            showConfigPanel_ = !showConfigPanel_;
        }
        inline bool wantCaptureKeyboard() 
        {
            // return configPanel_->wantCaptureKeyboard();
            return false;
        }
        inline bool wantCaptureMouse() 
        {
            // return configPanel_->wantCaptureMouse();
            return false;
        }
        inline void updateGestureZoom(float x, float y) 
        {
            orbitController_->zoomX = x;
            orbitController_->zoomY = y;
        }
        inline void updateGestureRotate(float x, float y) 
        {
            orbitController_->rotateX = x;
            orbitController_->rotateY = y;
        }
        inline void updateGesturePan(float x, float y) 
        {
            orbitController_->panX = x;
            orbitController_->panY = y;
        }

        [[nodiscard]] GLFWwindow* GetWindow() { return mWindow; }

    public:
        std::shared_ptr<Config> config_;
        std::shared_ptr<ConfigPanel> configPanel_;
        std::shared_ptr<Camera> camera_;
        std::shared_ptr<SmoothOrbitController> orbitController_;
        
        std::shared_ptr<EditorUI> editorUI_;
        std::shared_ptr<Viewer> viewer_;

        std::shared_ptr<ModelLoader> modelLoader_;

        GLFWwindow* mWindow;
        ImGuiLayer* mImGuiLayer;
        
        bool showConfigPanel_ = true;
        int rendererType_ = RENDER_TYPE_NONE;
        bool dumpFrame_ = false;
    };  
}