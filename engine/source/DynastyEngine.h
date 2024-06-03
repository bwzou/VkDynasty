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

#include "DynastyEditorUI.h"
#include "DynastyViewer.h"
#include "./render/FrameBuffer.h"



#define RENDER_TYPE_NONE (-1)

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class DynastyEngine {
public:
    bool initEngine(); 
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

