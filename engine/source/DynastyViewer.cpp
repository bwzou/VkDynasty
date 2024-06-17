#include "DynastyViewer.h"


bool DynastyViewer::iBLEnabled() {
    return false;
    // return config_.showSkybox && config_.pbrIbl && getSkyboxMaterial()->iblReady;
}

bool DynastyViewer::create(GLFWwindow* window, int width, int height, int outTexId) {
    cleanup();

    window_ = window;
    width_ = width;
    height_ = height;
    outTexId_ = outTexId;

    // main camera
    camera_ = &cameraMain_;

    // depth camera
    if (!cameraDepth_) {
        cameraDepth_ = std::make_shared<Camera>();
        cameraDepth_->setPerspective(glm::radians(CAMERA_FOV),
                                    (float) SHADOW_MAP_WIDTH / (float) SHADOW_MAP_HEIGHT,
                                    CAMERA_NEAR, CAMERA_FAR);
    }

    if (!renderer_) {
        renderer_ = createRenderer();
    }
    if (!renderer_) {
        throw std::runtime_error("DynastyViewer::create failed: createRenderer error");
        return false;
    }

    // create default resources
    uniformBlockScene_ = CREATE_UNIFORM_BLOCK(UniformsScene);
    uniformBlockModel_ = CREATE_UNIFORM_BLOCK(UniformsModel);
    uniformBlockMaterial_ = CREATE_UNIFORM_BLOCK(UniformsMaterial);

    shadowPlaceholder_ = createTexture2DDefault(1, 1, TextureFormat_FLOAT32, TextureUsage_Sampler);

    return true;
}


void DynastyViewer::destroy() {
    cleanup();
    if (renderer_) {
        renderer_->destroy();
    }
    renderer_ = nullptr;
}


std::shared_ptr<Renderer> DynastyViewer::createRenderer() {
    auto renderer = std::make_shared<Renderer>();
    if (!renderer->create(window_)) {
        return nullptr;
    }
    return renderer;
}


bool DynastyViewer::loadShaders(ShaderProgram &program, ShadingModel shading) {
    auto *programVK = dynamic_cast<ShaderProgram *>(&program);
    switch(shading) {
        CASE_CREATE_SHADER_VK(Shading_BaseColor, base);
        CASE_CREATE_SHADER_VK(Shading_BlinnPhong, blinn_phong);
        CASE_CREATE_SHADER_VK(Shading_PBR, pbr);
        CASE_CREATE_SHADER_VK(Shading_Skybox, skybox);
        CASE_CREATE_SHADER_VK(Shading_IBL_Irradiance, IBLIrradianceGLSL);
        CASE_CREATE_SHADER_VK(Shading_IBL_Prefilter, IBLPrefilterGLSL);
        CASE_CREATE_SHADER_VK(Shading_FXAA, FxaaGLSL);
      default:
        break;
    }
}


void DynastyViewer::waitRenderIdle() {
  if (renderer_) {
    renderer_->waitIdle();
  }
}


void DynastyViewer::drawFrame(DemoScene &scene) {
    LOG_INFO("=== DynastyViewer: start draw frame ===")
    if (!renderer_) {
        return;
    }

    renderer_->beginRender();

    scene_ = &scene;
    
    // setup framebuffer
    setupMainBuffers();
    setupShadowMapBuffers();

    // init skybox textures & ibl;
    // initSkyboxIBL();

    // setup model materials
    setupScene();

    // draw shadow map
    drawShadowMap();

    // setup fxaa
    // processFXAASetup();

    // main fxaa
    ClearStates clearStates{};
    clearStates.colorFlag = true;
    clearStates.depthFlag = config_.depthTest;
    clearStates.clearColor = config_.clearColor;
    clearStates.clearDepth = config_.reverseZ ? 0.f : 1.f;

    // start main pass
    renderer_->beginRenderPass(fboMain_, clearStates);
    renderer_->setViewPort(0, 0, width_, height_);
    
    // draw scene
    drawScene(false);

    // end main pass
    renderer_->endRenderPass();

    renderer_->endRender();

    // draw fxaa
    // processFXAADraw();

    LOG_INFO("=== DynastyViewer: end draw frame ===")
}


void DynastyViewer::setupScene() {
    LOG_INFO("=== DynastyViewer::setup scene ===")
    // point light
    if (config_.showLight) {
        setupPoints(scene_->pointLight);
    }

    // world axis
    if (config_.worldAxis) {
        setupLines(scene_->worldAxis);
    }

    // world axis
    if (config_.showFloor) {
        if (config_.wireframe) {
            setupMeshBaseColor(scene_->floor, true);
        } else {
            setupMeshTextured(scene_->floor);
        }
    }

    // model nodes
    ModelNode &modelNode = scene_->model->rootNode;
    setupModelNodes(modelNode, config_.wireframe);
}


void DynastyViewer::setupPoints(ModelPoints &points) {
    pipelineSetup(points, points.material->shadingModel, {UniformBlock_Model, UniformBlock_Material});
}


void DynastyViewer::setupLines(ModelLines &lines) {
    pipelineSetup(lines, lines.material->shadingModel, {UniformBlock_Model, UniformBlock_Material});
}


void DynastyViewer::setupMeshBaseColor(ModelMesh &mesh, bool wireframe) {
    pipelineSetup(mesh, Shading_BaseColor, {UniformBlock_Model, UniformBlock_Scene, UniformBlock_Material},
                [&](RenderStates &rs) -> void {
                  rs.polygonMode = wireframe ? PolygonMode_LINE : PolygonMode_FILL;  
                });
    // pipelineSetup(mesh, Shading_BlinnPhong, {UniformBlock_Model, UniformBlock_Scene, UniformBlock_Material});
}


void DynastyViewer::setupMeshTextured(ModelMesh &mesh) {
    pipelineSetup(mesh, mesh.material->shadingModel, {UniformBlock_Model, UniformBlock_Scene, UniformBlock_Material});
}


void DynastyViewer::setupModelNodes(ModelNode &node, bool wireframe) {
    for (auto &mesh : node.meshes) {
        if (wireframe) {
            setupMeshBaseColor(mesh, true);
        } else {
            setupMeshTextured(mesh);
        }
    }

    // setup child
    for (auto &childNode : node.children) {
        setupModelNodes(childNode, wireframe);
    }
}


void DynastyViewer::setupSkybox(ModelMesh &skybox) {

}


void DynastyViewer::drawShadowMap() {
    // 绘制阴影的时候，我们需要先绘制一遍获得一个深度图，然后通过深度图作为输入，再绘制物体，绘制物体的时候通过深度图计算最终的颜色。
    LOG_INFO("=== DynastyViewer::draw shadow map ===")


    // main fxaa
    ClearStates clearDepth{};
    clearDepth.depthFlag = true;
    clearDepth.clearDepth = config_.reverseZ ? 0.f : 1.f;

    renderer_->beginRenderPass(fboShadow_, clearDepth);
    renderer_->setViewPort(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

    // set camera
    cameraDepth_->lookAt(config_.pointLightPosition, glm::vec3(0), glm::vec3(0, 1, 0));
    cameraDepth_->update();
    camera_ = cameraDepth_.get();

    // draw scene
    drawScene(true);

    renderer_->endRenderPass();
   
    // set back to main camera
    camera_ = &cameraMain_;
}   


void DynastyViewer::drawScene(bool shadowPass) {
    LOG_INFO("=== DynastyViewer::draw scene ===")
    updateUniformScene();
    updateUniformModel(glm::mat4(1.0f), camera_->viewMatrix());

    // draw point light
    if (!shadowPass && config_.showLight) {
        LOG_INFO("draw point light");
        updateUniformMaterial(*scene_->pointLight.material);
        pipelineDraw(scene_->pointLight);
    }

    // draw world axis 
    if (!shadowPass && config_.worldAxis) {
        LOG_INFO("draw world axis");
        updateUniformMaterial(*scene_->worldAxis.material);
        pipelineDraw(scene_->worldAxis);
    }

    // draw floor
    if (!shadowPass && config_.showFloor) {
        LOG_INFO("draw floor");
       drawModelMesh(scene_->floor, shadowPass, 0.f);
    }

    LOG_INFO("draw model");
    // draw model nodes opaque
    ModelNode &modelNode = scene_->model->rootNode;
    count = 0;
    drawModelNodes(modelNode, shadowPass, scene_->model->centeredTransform, Alpha_Opaque);
    LOG_INFO("Alpha_Opaque model nodes: {}", count)

    // draw model nodes blend
    count = 0;
    drawModelNodes(modelNode, shadowPass, scene_->model->centeredTransform, Alpha_Blend);
    LOG_INFO("Alpha_Blend model nodes: {}", count)
}


void DynastyViewer::drawModelNodes(ModelNode &node, bool shadowPass, glm::mat4 &transform, AlphaMode mode, float specular) {
    glm::mat4 modelMatrix = transform * node.transform;

    // update model uniform
    updateUniformModel(modelMatrix, camera_->viewMatrix());

    // draw nodes
    for (auto &mesh : node.meshes) {
        if (mesh.material->alphaMode != mode) {
            continue;
        }

        // frustum cull 四棱锥剔除
        // if (!checkMeshFrustumCull(mesh, modelMatrix)) {
        //     return;
        // }
        
        count ++;
        drawModelMesh(mesh, shadowPass, specular);
    }

    // draw child
    for (auto &childNode : node.children) {
        drawModelNodes(childNode, shadowPass, modelMatrix, mode, specular);
    }
}


void DynastyViewer::drawModelMesh(ModelMesh &mesh, bool shadowPass, float specular) {
    // update material
    updateUniformMaterial(*mesh.material, specular);

    // update shadow textures
    if (config_.shadowMap) {
        updateShadowTextures(mesh.material->materialObj.get(), shadowPass);
    }

    // draw mesh
    pipelineDraw(mesh);
}


void DynastyViewer::pipelineSetup(ModelBase &model, ShadingModel shading, const std::set<int> &uniformBlocks,
                           const std::function<void(RenderStates &rs)> &extraStates) {
    setupVertexArray(model);

    // reset materialObj if ShadingModel changed
    if (model.material->materialObj != nullptr) {
        if (model.material->materialObj->shadingModel != shading) {
            model.material->materialObj = nullptr;
        }
    }
    setupMaterial(model, shading, uniformBlocks, extraStates);
}


void DynastyViewer::pipelineDraw(ModelBase &model) {
    auto &materialObj = model.material->materialObj;

    renderer_->setVertexArrayObject(model.vao);
    renderer_->setShaderProgram(materialObj->shaderProgram);  
    renderer_->setShaderResources(materialObj->shaderResources);
    renderer_->setPipelineStates(materialObj->pipelineStates);
    renderer_->draw();
}
    

void DynastyViewer::setupMainBuffers() {
    if (config_.aaType == AAType_MSAA) {
        setupMainColorBuffer(true);
        setupMainDepthBuffer(true);
    } else {
        setupMainColorBuffer(false);
        setupMainDepthBuffer(false);
    }

    if (!fboMain_) {
        fboMain_ = renderer_->createFrameBuffer(false);
    }
    fboMain_->setColorAttachment(texColorMain_, 0);
    fboMain_->setDepthAttachment(texDepthMain_);
    fboMain_->setOffscreen(false);

    if (!fboMain_->isValid()) {
        LOG_ERROR("+++ setupMainBuffers failed +++");
    }
}


void DynastyViewer::setupShadowMapBuffers() {
    LOG_INFO("DynastyViewer: setup shadow map buffers");
    if (!config_.shadowMap) {
        return;
    }

    if (!fboShadow_) {
        fboShadow_ = renderer_->createFrameBuffer(true);
    }

    if (!texDepthShadow_) {
        TextureDesc texDesc{};
        // texDesc.width = SHADOW_MAP_WIDTH;
        // texDesc.height = SHADOW_MAP_HEIGHT;
        texDesc.width = renderer_->getVkCtx().swapChainExtent().width;
        texDesc.height = renderer_->getVkCtx().swapChainExtent().height;
        texDesc.type = TextureType_2D;
        texDesc.format = TextureFormat_FLOAT32;
        texDesc.usage = TextureUsage_Sampler | TextureUsage_AttachmentDepth;
        texDesc.useMipmaps = false;
        texDesc.multiSample = false;
        texDepthShadow_ = renderer_->createTexture(texDesc);

        SamplerDesc sampler{};
        sampler.filterMin = Filter_NEAREST;
        sampler.filterMag = Filter_NEAREST;
        sampler.wrapS = Wrap_CLAMP_TO_BORDER;
        sampler.wrapT = Wrap_CLAMP_TO_BORDER;
        sampler.borderColor = config_.reverseZ ? Border_BLACK : Border_WHITE;
        texDepthShadow_->setSamplerDesc(sampler);

        texDepthShadow_->initImageData();
        fboShadow_->setDepthAttachment(texDepthShadow_); // 获取深度图

        if (!fboShadow_->isValid()) {
            LOG_ERROR("+++ setupShadowMapBuffers failed +++");
        }
    }
}


void DynastyViewer::setupMainColorBuffer(bool multiSample) {

}   


void DynastyViewer::setupMainDepthBuffer(bool multiSample) {
    if (!texDepthMain_ || texDepthMain_->multiSample != multiSample) {
        TextureDesc texDesc{};
        texDesc.width = renderer_->getVkCtx().swapChainExtent().width;
        texDesc.height = renderer_->getVkCtx().swapChainExtent().height;
        texDesc.type = TextureType_2D;
        texDesc.format = TextureFormat_FLOAT32;
        texDesc.usage = TextureUsage_AttachmentDepth;
        texDesc.useMipmaps = false;
        texDesc.multiSample = multiSample;
        texDepthMain_ = renderer_->createTexture(texDesc);

        SamplerDesc sampler{};
        sampler.filterMin = Filter_NEAREST;
        sampler.filterMag = Filter_NEAREST;
        texDepthMain_->setSamplerDesc(sampler);

        texDepthMain_->initImageData();
    }
}


void DynastyViewer::setupVertexArray(ModelVertexes &vertexes) {
    LOG_INFO("=== DynastyViewer::setup vertex array ===")
    if (!vertexes.vao) {
        vertexes.vao = renderer_->createVertexArrayObject(vertexes);
    }
}


void DynastyViewer::setupSamplerUniforms(Material &material) {
    for (auto &kv: material.textures) {
        // create sampler uniform
        const char * samplerName = Material::samplerName((MaterialTexType) kv.first);
        std::cout << samplerName << std::endl;
        if (samplerName) {
            auto uniform = renderer_->createUniformSampler(samplerName, *kv.second);
            uniform->setTexture(kv.second);
            material.materialObj->shaderResources->samplers[kv.first] = std::move(uniform);
        }
    }
}


bool DynastyViewer::setupShaderProgram(Material &material, ShadingModel shading) {
    size_t cacheKey = getShaderProgramCacheKey(shading, material.shaderDefines);

    auto cacheProgram = programCache_.find(cacheKey);
    if (cacheProgram != programCache_.end()) { 
        material.materialObj->shaderProgram = cacheProgram->second;
        material.materialObj->shaderResources = std::make_shared<ShaderResources>();
        return true;
    }

    auto program = renderer_->createShaderProgram();

    bool success = loadShaders(*program, shading);
    if (success) {
        // add  to cache
        programCache_[cacheKey] = program;
        material.materialObj->shaderProgram = program;
        material.materialObj->shaderResources = std::make_shared<ShaderResources>();
    } else {
        throw std::runtime_error("setupShaderProgram failed!");
    }

    return success;
}


void DynastyViewer::setupPipelineStates(ModelBase &model, const std::function<void(RenderStates &rs)> &extraStates) {
    auto &material = *model.material;
    RenderStates rs;
    rs.blend = material.alphaMode == Alpha_Blend;
    rs.blendParams.SetBlendFactor(BlendFactor_SRC_ALPHA, BlendFactor_ONE_MINUS_SRC_ALPHA);

    rs.depthTest = config_.depthTest;
    rs.depthMask = !rs.blend; // disable depth write if blending enabled
    rs.depthFunc = config_.reverseZ ? DepthFunc_GREATER : DepthFunc_LESS;
    
    rs.cullFace = config_.cullFace && (!material.doubleSided);

    rs.primitiveType = model.primitiveType;
    rs.polygonMode = PolygonMode_FILL;

    rs.lineWidth = material.lineWidth;

    if (extraStates) {
        extraStates(rs);
    }

    size_t cacheKey = getPipelineCacheKey(material, rs);
    auto it = pipelineCache_.find(cacheKey);
    if (it != pipelineCache_.end()) {
        material.materialObj->pipelineStates = it->second;
    } else {
        auto pipelineStates = renderer_->createPipelineStates(rs);
        material.materialObj->pipelineStates = pipelineStates;
        pipelineCache_[cacheKey] = pipelineStates;
    }
}


void DynastyViewer::setupMaterial(ModelBase &model, ShadingModel shading, const std::set<int> &uniformBlocks,
                           const std::function<void(RenderStates &rs)> &extraStates) {
    auto &material = *model.material;

    if (material.textures.empty()) {
        setupTextures(material);
    }

    // setup uniform && samplers
    if (!material.materialObj) {
        material.materialObj = std::make_shared<MaterialObject>();
        material.materialObj->shadingModel = shading;

        if (setupShaderProgram(material, shading)) {
            setupSamplerUniforms(material);
        }

        // setup uniform blocks
        for(auto &key : uniformBlocks) {
            std::shared_ptr<UniformBlock> uniform = nullptr;
            switch (key) {
                case UniformBlock_Scene: {
                    uniform = uniformBlockScene_;
                    break;
                }
                case UniformBlock_Model: {
                    uniform = uniformBlockModel_;
                    break;
                }
                case UniformBlock_Material: {
                    uniform = uniformBlockMaterial_;
                    break;
                }
                default:
                    break;
            }
            if (uniform) {
                material.materialObj->shaderResources->blocks[key] = uniform;
            }
        }
    }

    setupPipelineStates(model, extraStates);
}


void DynastyViewer::setupTextures(Material &material) {
    for (auto &kv : material.textureData) {
        TextureDesc texDesc{};
        texDesc.width = (int) kv.second.width;
        texDesc.height = (int) kv.second.height;
        texDesc.format = TextureFormat_RGBA8;
        texDesc.usage = TextureUsage_Sampler | TextureUsage_UploadData;
        texDesc.useMipmaps = false;
        texDesc.multiSample = false;

        SamplerDesc sampler{};
        sampler.wrapS = kv.second.wrapModeU;
        sampler.wrapT = kv.second.wrapModeV;
        sampler.filterMin = Filter_LINEAR;
        sampler.filterMag = Filter_LINEAR;

        std::shared_ptr<Texture> texture = nullptr;
        switch(kv.first) {
            case MaterialTexType_IBL_IRRADIANCE:
            case MaterialTexType_IBL_PREFILTER: {
                break;
            }
            case MaterialTexType_CUBE: {
                texDesc.type = TextureType_CUBE;
                sampler.wrapR = kv.second.wrapModeW;
            }
            default: {
                texDesc.type = TextureType_2D;
                texDesc.useMipmaps = config_.mipmaps;
                sampler.filterMin =  config_.mipmaps ? Filter_LINEAR_MIPMAP_LINEAR : Filter_LINEAR;
                break;
            }
        }
        texture = renderer_->createTexture(texDesc);
        texture->setSamplerDesc(sampler);
        texture->setImageData(kv.second.data);
        texture->tag = kv.second.tag;
        material.textures[kv.first] = texture;
    }

    // default shadow depth texture
    if (material.shadingModel != Shading_Skybox) {
        material.textures[MaterialTexType_SHADOWMAP]=  shadowPlaceholder_;
    }
}


void DynastyViewer::updateUniformScene() {
    static UniformsScene uniformsScene{};

    uniformsScene.u_ambientColor = config_.ambientColor;
    uniformsScene.u_cameraPosition = camera_->eye();
    uniformsScene.u_pointLightPosition = config_.pointLightPosition;
    uniformsScene.u_pointLightColor = config_.pointLightColor;

    uniformBlockScene_->setData(&uniformsScene, sizeof(UniformsScene));
}


void DynastyViewer::updateUniformModel(const glm::mat4 &model, const glm::mat4 &view) {
    static UniformsModel uniformsModel{};

    uniformsModel.u_reverseZ = config_.reverseZ ? 1u : 0u;
    uniformsModel.u_modelMatrix = model;
    uniformsModel.u_modelViewProjectionMatrix = camera_->projectionMatrix() * view * model;
    uniformsModel.u_inverseTransposeModelMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

    // shadow mvp
    if (config_.shadowMap && cameraDepth_) {
        const glm::mat4 biasMat = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 0.5f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.0f,
                                        0.5f, 0.5f, 0.0f, 1.0f);
        uniformsModel.u_shadowMVPMatrix = biasMat * cameraDepth_->projectionMatrix() * cameraDepth_->viewMatrix() * model;                                
    }

    uniformBlockModel_->setData(&uniformsModel, sizeof(UniformsModel));
}


void DynastyViewer::updateUniformMaterial(Material &material, float specular) {
    static UniformsMaterial uniformsMaterial{};

    uniformsMaterial.u_enableLight = config_.showLight ? 1u : 0u;
    uniformsMaterial.u_enableIBL = iBLEnabled() ? 1u : 0u;
    uniformsMaterial.u_enableShadow = config_.shadowMap ? 1u : 0u;

    uniformsMaterial.u_pointSize = material.pointSize;
    uniformsMaterial.u_kSpecular = specular;
    uniformsMaterial.u_baseColor = material.baseColor;

    uniformBlockMaterial_->setData(&uniformsMaterial, sizeof(UniformsMaterial));
}


void DynastyViewer::updateShadowTextures(MaterialObject *materialObj, bool shadowPass) {
    if (!materialObj->shaderResources) {
        return;
    }

    auto &samplers = materialObj->shaderResources->samplers;
    if (shadowPass) {
        samplers[MaterialTexType_SHADOWMAP]->setTexture(shadowPlaceholder_);
    } else {
        samplers[MaterialTexType_SHADOWMAP]->setTexture(texDepthShadow_);
    }
}


std::shared_ptr<Texture> DynastyViewer::createTexture2DDefault(int width, int height, TextureFormat format, uint32_t usage, bool mipmaps) {
    TextureDesc texDesc{};
    texDesc.width = width;
    texDesc.height = height;
    texDesc.type = TextureType_2D;
    texDesc.format = format;
    texDesc.usage = usage;
    texDesc.useMipmaps = mipmaps;
    texDesc.multiSample = false;
    auto texture2d = renderer_->createTexture(texDesc);
    if (!texture2d) {
        return nullptr;
    }

    SamplerDesc sampler{};
    sampler.filterMin = mipmaps ? Filter_LINEAR_MIPMAP_LINEAR : Filter_LINEAR;
    sampler.filterMag = Filter_LINEAR;
    texture2d->setSamplerDesc(sampler);

    texture2d->initImageData();
    return texture2d;
}


size_t DynastyViewer::getShaderProgramCacheKey(ShadingModel shadingModel, std::set<std::string> shaderDefines) {
    size_t seed = 0;
    HashUtils::hashCombine(seed, (int) shadingModel);
    for (auto &def : shaderDefines) {
        HashUtils::hashCombine(seed, def);
    }
    return seed;
}


size_t DynastyViewer::getPipelineCacheKey(Material &material, const RenderStates &rs) {
    size_t seed = 0;

    HashUtils::hashCombine(seed, (int) material.materialObj->shadingModel);

    // TODO pack together
    HashUtils::hashCombine(seed, rs.blend);
    HashUtils::hashCombine(seed, (int) rs.blendParams.blendFuncRgb);
    HashUtils::hashCombine(seed, (int) rs.blendParams.blendSrcRgb);
    HashUtils::hashCombine(seed, (int) rs.blendParams.blendDstRgb);
    HashUtils::hashCombine(seed, (int) rs.blendParams.blendFuncAlpha);
    HashUtils::hashCombine(seed, (int) rs.blendParams.blendSrcAlpha);
    HashUtils::hashCombine(seed, (int) rs.blendParams.blendDstAlpha);

    HashUtils::hashCombine(seed, rs.depthTest);
    HashUtils::hashCombine(seed, rs.depthMask);
    HashUtils::hashCombine(seed, (int) rs.depthFunc);

    HashUtils::hashCombine(seed, rs.cullFace);
    HashUtils::hashCombine(seed, (int) rs.primitiveType);
    HashUtils::hashCombine(seed, (int) rs.polygonMode);

    HashUtils::hashCombine(seed, rs.lineWidth);

    return seed;
}