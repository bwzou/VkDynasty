#pragma once

#include <string>
#include "runtime/code/base/GLMInc.h"
// #include "../render/Renderer.h"

namespace DynastyEngine
{
    const std::string ASSETS_DIR = "/Users/bowenzou/Workspace/Projects/dynasty/engine/asset/";
    const std::string SHADER_GLSL_DIR = "/Users/bowenzou/Workspace/Projects/dynasty/engine/shader/generated/spv/";
    const std::string SHADER_JSON_DIR = "/Users/bowenzou/Workspace/Projects/dynasty/engine/shader/json/";

    enum AAType 
    {
        AAType_NONE,
        AAType_MSAA,
        AAType_FXAA,
    };

    enum Mode 
    {
        SM,
        PCSS,
        VSM,
        CSM
    };

    class Config 
    {
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
        size_t shadowMapType = 0; // 0 SM; 1 PCSS; 2 VSM; 3 CSM

        bool pbrIbl = false;
        bool mipmaps = false;

        bool cullFace = true;
        bool depthTest = true;
        bool reverseZ = false;

        glm::vec4 clearColor = {0.f, 0.f, 0.f, 0.f};
        glm::vec3 ambientColor = {0.2f, 0.2f, 0.2f};

        bool showLight = true;
        // glm::vec3 pointLightPosition = {0.f, 0.f, 0.f};
        glm::vec3 pointLightColor = {0.5f, 0.5f, 0.5f};
        glm::vec3 pointLightPosition = {1.942f, 2.796f, 0.7738f};
        // glm::vec3 pointLightColor = {1.f, 1.f, 1.f};

        int aaType = AAType_NONE;
        // int rendererType = Renderer_SOFT;
        int rendererType = 0; // 用哪种技术渲染

        bool showSSR= false;
        
    };
}
