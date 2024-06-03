#pragma once

#include <vector>
#include <iostream>
#include <functional>
#include <unordered_map>
#include "Config.h"
#include "../code/util/FileUtils.h"

class ConfigPanel {
public:
    explicit ConfigPanel(Config &config) : config_(config) {}
    ~ConfigPanel() { destroy(); };

    bool init(void* window, int width, int height);
    void onDraw();
    
    void update();
    void updateSize(int width, int height);

    inline void setReloadModelFunc(const std::function<bool(const std::string &)> &func) {
        reloadModelFunc_ = func;
    }


private:
    bool loadConfig();
    bool reloadModel(const std::string &name);
    bool reloadSkybox(const std::string &name);

    void drawSettings();
    void destroy();


private:
    Config &config_;
    
    int frameWidth_ = 0;
    int frameHeight_ = 0;

    std::unordered_map<std::string, std::string> modelPaths_;
    std::unordered_map<std::string, std::string> skyboxPaths_;

    std::vector<const char *> modelNames_;
    std::vector<const char *> skyboxNames_;

    std::function<bool(const std::string &path)> reloadModelFunc_;
};
