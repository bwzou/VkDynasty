#pragma once

#include <string>
#include "../code/base/GLMInc.h"
// #include "../render/Renderer.h"

const std::string ASSETS_DIR = "../../asset/";
const std::string SHADER_GLSL_DIR = "../../shader/generated/spv";

enum AAType {
    AAType_NONE,
    AAType_MSAA,
    AAType_FXAA,
};

class Config {
public:
    std::string modelName;
    std::string modelPath;
    std::string skyboxName;
    std::string skyboxPath;

    size_t triangleCount_ = 0;
    bool wireframe = false;
    bool worldAxis = true;
    bool showSkybox = false;
    bool showFloor = true;

    bool shadowMap = true;
    bool pbrIbl = false;
    bool mipmaps = false;

    bool cullFace = true;
    bool depthTest = true;
    bool reverseZ = false;

    glm::vec4 clearColor = {0.f, 0.f, 0.f, 0.f};
    glm::vec3 ambientColor = {0.5f, 0.5f, 0.5f};

    bool showLight = true;
    glm::vec3 pointLightPosition = {0.f, 0.f, 0.f};
    glm::vec3 pointLightColor = {0.5f, 0.5f, 0.5f};

    int aaType = AAType_NONE;
    // int rendererType = Renderer_SOFT;
    int rendererType = 0; // 用那种技术渲染
};


