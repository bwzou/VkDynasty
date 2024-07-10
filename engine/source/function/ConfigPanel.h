#pragma once

#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <functional>
#include <unordered_map>
#include "Config.h"
#include "../render/Renderer.h"
#include "../code/util/FileUtils.h"


class ConfigPanel {
public:
    explicit ConfigPanel(Config &config) : config_(config) {}
    ~ConfigPanel() { destroy(); };

    bool init(void* window, int width, int height);
    bool initImgui(void *window, std::shared_ptr<Renderer> renderer);
    void onDraw();
    
    void update();
    void updateSize(int width, int height);

    bool wantCaptureKeyboard();
    bool wantCaptureMouse();

    inline void setReloadModelFunc(const std::function<bool(const std::string &)> &func) {
        reloadModelFunc_ = func;
    }
    inline void setUpdateLightFunc(const std::function<void(glm::vec3 &, glm::vec3 &)> &func) {
        updateLightFunc_ = func;
    }
    inline bool initialize() {
        return initialize_;
    }

private:
    bool loadConfig();
    bool reloadModel(const std::string &name);
    bool reloadSkybox(const std::string &name);

    void drawSettings();
    void destroy();


private:
    Config &config_;
    Renderer* renderer_;
    VkRenderPass imguiRenderPass_;
    
    int frameWidth_ = 0;
    int frameHeight_ = 0;

    float lightPositionAngle_ = glm::radians(120.f);

    bool initialize_ = false;

    std::unordered_map<std::string, std::string> modelPaths_;
    std::unordered_map<std::string, std::string> skyboxPaths_;

    std::vector<const char *> modelNames_;
    std::vector<const char *> skyboxNames_;

    std::function<bool(const std::string &path)> reloadModelFunc_;
    std::function<void(glm::vec3 &position, glm::vec3 &color)> updateLightFunc_;

    int smType = 0;
};
