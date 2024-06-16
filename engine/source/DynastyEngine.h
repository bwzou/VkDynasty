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
#include "./function/Model.h"
#include "./function/Material.h"
#include "./function/Config.h"
#include "./function/ConfigPanel.h"
#include "./function/Camera.h"
#include "./function/OrbitController.h"
#include "./function/ModelLoader.h"
#include "./code/util/FileUtils.h"
#include "./code/base/macro.h"

#include "DynastyEditorUI.h"
#include "DynastyViewer.h"
#include "./render/FrameBuffer.h"


#define RENDER_TYPE_NONE (-1)

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;


class DynastyEngine {
public:
    bool initEngine(); 
    bool initEvent(GLFWwindow* window); 
    bool run();
    void drawFrame();
    void setupConfigPanelActions();
    void cleanup();

    inline void waitRenderIdle() {
        if (rendererType_ != RENDER_TYPE_NONE) {
            viewer_->waitRenderIdle();
        }
    }
    inline void resetStates() {
        waitRenderIdle();
        modelLoader_->resetAllModelStates();
        modelLoader_->getScene().resetStates();
    }
    inline void drawPanel() {
        if (showConfigPanel_) {
            configPanel_->onDraw();
        }
    }
    inline void togglePanelState() {
        showConfigPanel_ = !showConfigPanel_;
    }
    inline bool wantCaptureKeyboard() {
        return configPanel_->wantCaptureKeyboard();
    }
    inline bool wantCaptureMouse() {
        return configPanel_->wantCaptureMouse();
    }
    inline void updateGestureZoom(float x, float y) {
        orbitController_->zoomX = x;
        orbitController_->zoomY = y;
    }
    inline void updateGestureRotate(float x, float y) {
        orbitController_->rotateX = x;
        orbitController_->rotateY = y;
    }
    inline void updateGesturePan(float x, float y) {
        orbitController_->panX = x;
        orbitController_->panY = y;
    }

public:
    std::shared_ptr<Config> config_;
    std::shared_ptr<ConfigPanel> configPanel_;
    std::shared_ptr<Camera> camera_;
    std::shared_ptr<SmoothOrbitController> orbitController_;
    
    std::shared_ptr<DynastyEditorUI> editorUI_;
    std::shared_ptr<DynastyViewer> viewer_;

    std::shared_ptr<ModelLoader> modelLoader_;
    
    bool showConfigPanel_ = true;
    int rendererType_ = RENDER_TYPE_NONE;
    bool dumpFrame_ = false;
};  