#include "ConfigPanel.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "json11.hpp"


bool ConfigPanel::init(void *window, int width, int height) {
    frameWidth_ = width;
    frameHeight_ = height;

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle *style = &ImGui::GetStyle();
    style->Alpha = 0.8f;

    // Setup Platform/Renderer backends
    // ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *) window, true);
    // ImGui_ImplOpenGL3_Init("#version 330 core");

    // load config
    return loadConfig();
}


void ConfigPanel::onDraw() {
    // Start the ImGui frame
    // ImGui_ImplOpenGL3_NewFrame();
    // ImGui_ImplGlfw_NewFrame();
    std::cout << "Start the ImGui frame" << std::endl;
    ImGui::NewFrame();

    // Settings window
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    drawSettings();

    ImGui::SetWindowPos(ImVec2(frameWidth_ - ImGui::GetWindowWidth(), 0));
    ImGui::End();
    ImGui::Render();
}


void ConfigPanel::drawSettings() {
    
}


void ConfigPanel::update() {

}
    

void ConfigPanel::updateSize(int width, int height) {
    frameWidth_ = width;
    frameHeight_ = height;
}


bool ConfigPanel::loadConfig() {
    auto configPath = ASSETS_DIR + "assets.json";
    auto configStr = FileUtils::readText(configPath);
    
    if (configStr.empty()) {
        throw std::runtime_error("configPath is error!");
        return false;
    }

    std::string err;
    const auto json = json11::Json::parse(configStr, err);
    for (auto &kv : json["model"].object_items()) {
        modelPaths_[kv.first] = ASSETS_DIR + kv.second["path"].string_value();
    }
    // for (auto &kv :  json["skybox"].object_items()) {
    //   skyboxPaths_[kv.first] = ASSETS_DIR + kv.second["path"].string_value();
    // }
    
    if (modelPaths_.empty()) { 
       std::cout << "load models failed: %s" << err.c_str() << std::endl;
       return false;
    }

    for (const auto &kv : modelPaths_) {
        modelNames_.emplace_back(kv.first.c_str());
    }
    // for (const auto &kv : skyboxPaths_) {
    //     skyboxPaths_.emplace_back(kv.first.c_str());
    // }
    return reloadModel(modelPaths_.begin()->first);
}


bool ConfigPanel::reloadModel(const std::string &name) {
    if (name != config_.modelName) {
        config_.modelName = name;
        config_.modelPath = modelPaths_[config_.modelName];

        if (reloadModelFunc_) {
            return reloadModelFunc_(config_.modelPath);
        }
    }
    return true;
}


bool ConfigPanel::reloadSkybox(const std::string &name) {

}


void ConfigPanel::destroy() {

}





