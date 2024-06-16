#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif
#include <GLFW/glfw3.h>
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
#include <set>
#include <unordered_map>

#include "./function/Model.h"
#include "./function/Material.h"
#include "./function/Config.h"
#include "./function/Camera.h"
#include "./function/ModelLoader.h"
#include "./code/util/FileUtils.h"
#include "./code/util/HashUtils.h"
#include "./code/base/macro.h"

#include "./render/FrameBuffer.h"
#include "./render/Renderer.h"
#include "./render/VulkanContext.h"

#define SHADOW_MAP_WIDTH 512
#define SHADOW_MAP_HEIGHT 512

#define CASE_CREATE_SHADER_VK(shading, source) case shading: \
  return programVK->compileAndLinkGLSLFile(SHADER_GLSL_DIR + #source + ".vert.spv", \
                                           SHADER_GLSL_DIR + #source + ".frag.spv", #source)

#define CREATE_UNIFORM_BLOCK(name) renderer_->createUniformBlock(#name, sizeof(name))

class DynastyViewer {
public:
    DynastyViewer(Config &config, Camera &camera) : config_(config), cameraMain_(camera) {}

    bool create(GLFWwindow* window, int width, int height, int outTexId);
    void destroy();

    std::shared_ptr<Renderer> createRenderer();
    bool loadShaders(ShaderProgram &program, ShadingModel shading);
    void drawFrame(DemoScene &scene);

    void waitRenderIdle();

    void configRenderer() {
        camera_->setReverseZ(config_.reverseZ);
        cameraDepth_->setReverseZ(config_.reverseZ);
    }

    std::shared_ptr<Renderer> getRenderer() {
        return renderer_;
    }

    void* getDevicePointer() {
        return (*((void **) (renderer_->getVkCtx().instance())));
    }

    inline Config getConfig() {
        return config_;
    }

private:
    bool iBLEnabled();

    void setupScene();
    void setupPoints(ModelPoints &points);
    void setupLines(ModelLines &lines);
    void setupMeshBaseColor(ModelMesh &mesh, bool wireframe);
    void setupMeshTextured(ModelMesh &mesh);
    void setupModelNodes(ModelNode &node, bool wireframe);
    void setupSkybox(ModelMesh &skybox);

    void drawScene(bool shadowPass);
    void drawModelNodes(ModelNode &node, bool shadowPass, glm::mat4 &tranform, AlphaMode mode, float specular = 1.f);
    void drawModelMesh(ModelMesh &mesh, bool shadowPass, float specular);

    void pipelineSetup(ModelBase &model, ShadingModel shading, const std::set<int> &uniformBlocks,
                        const std::function<void(RenderStates &rs)> &extraStates = nullptr);
    void pipelineDraw(ModelBase &model);
    
    void setupMainBuffers();
    void setupShadowMapBuffers();
    void setupMainColorBuffer(bool multiSample);
    void setupMainDepthBuffer(bool multiSample);
    void setupVertexArray(ModelVertexes &vertexes);
    void setupSamplerUniforms(Material &material);
    bool setupShaderProgram(Material&material, ShadingModel shading);
    void setupPipelineStates(ModelBase &model, const std::function<void(RenderStates &rs)> &extraStates);
    void setupMaterial(ModelBase &model, ShadingModel shading, const std::set<int> &uniformBlocks, const std::function<void(RenderStates &rs)> &extraStates);
    void setupTextures(Material &material); 

    void updateUniformScene();
    void updateUniformModel(const glm::mat4 &model, const glm::mat4 &view);
    void updateUniformMaterial(Material &material, float specular = 1.f);

    static size_t getShaderProgramCacheKey(ShadingModel shadingModel, std::set<std::string> shaderDefines);
    size_t getPipelineCacheKey(Material &material, const RenderStates &rs);

    void cleanup() {
        if (renderer_) {
            renderer_->waitIdle();
        }
        fboMain_ = nullptr;
        texColorMain_ = nullptr;
        texDepthMain_ = nullptr;
        // fboShadow_ = nullptr;
        // texDepthShadow_ = nullptr;
        // shadowPlaceholder_ = nullptr;
        // fxaaFilter_ = nullptr;
        // texColorFxaa_ = nullptr;
        // iblPlaceholder_ = nullptr;
        // iblGenerator_ = nullptr;
        uniformBlockScene_ = nullptr;
        uniformBlockModel_ = nullptr;
        uniformBlockMaterial_ = nullptr;
        programCache_.clear();
        pipelineCache_.clear();
    }


public:
    int count = 0;
    Config &config_;

    Camera &cameraMain_;
    std::shared_ptr<Camera> cameraDepth_ = nullptr;
    Camera *camera_ = nullptr;
    
    // std::shared_ptr<DynastyEditorUI> editorUI_;
    // std::shared_ptr<VulkanContext> vulkanContext;
    // std::shared_ptr<ModelLoader> modelLoader;
    
    // std::unordered_map<std::string, std::string> modelPaths;
    // std::unordered_map<std::string, std::string> skyboxPaths;

    // std::vector<const char *> modelNames;
    // std::vector<const char *> skyboxNames;

    GLFWwindow* window_;
    int width_ = 0;
    int height_ = 0;
    int outTexId_ = 0;

    DemoScene *scene_ = nullptr;
    std::shared_ptr<Renderer> renderer_ = nullptr;

    // main fbo
    std::shared_ptr<FrameBuffer> fboMain_ = nullptr;
    std::shared_ptr<Texture> texColorMain_ = nullptr;
    std::shared_ptr<Texture> texDepthMain_ = nullptr;

    // shadow map

    // uniforms
    std::shared_ptr<UniformBlock> uniformBlockScene_;
    std::shared_ptr<UniformBlock> uniformBlockModel_;
    std::shared_ptr<UniformBlock> uniformBlockMaterial_;

    // caches
    std::unordered_map<size_t, std::shared_ptr<ShaderProgram>> programCache_;
    std::unordered_map<size_t, std::shared_ptr<PipelineStates>> pipelineCache_;
};