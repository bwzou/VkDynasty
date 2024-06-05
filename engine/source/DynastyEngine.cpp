#include "DynastyEngine.h"


void DynastyEngine::drawFrame() {
    orbitController_->update();
    camera_->update();
    configPanel_->update();


    // update triangle count
    config_->triangleCount_ = modelLoader_->getModelPrimitiveCnt();
    
    // viewer_->create(SCR_WIDTH, SCR_HEIGHT, outTexId_);
    viewer_->create(editorUI_->window(), SCR_WIDTH, SCR_HEIGHT, NULL);
    
    // viewer_->configRenderer();
    viewer_->drawFrame(modelLoader_->getScene());

    // viewer_->swapBuffer();
    std::cout << "viewer swap" << std::endl;
}

bool DynastyEngine::initEngine() {
    // camera
    camera_ = std::make_shared<Camera>();
    camera_->setPerspective(glm::radians(CAMERA_FOV), (float) SCR_WIDTH / (float) SCR_HEIGHT, CAMERA_NEAR, CAMERA_FAR);

    // orbit controller
    orbitController_ = std::make_shared<SmoothOrbitController>(std::make_shared<OrbitController>(*camera_));

    config_ = std::make_shared<Config>();
    configPanel_ = std::make_shared<ConfigPanel>(*config_);

    editorUI_ = std::make_shared<DynastyEditorUI>(*config_);
    editorUI_ ->initWindow();

    viewer_ = std::make_shared<DynastyViewer>(*config_, *camera_);
        
    modelLoader_ = std::make_shared<ModelLoader>(*config_);
    // setup config panel actions callback
    setupConfigPanelActions();

    // init config
    configPanel_->init(editorUI_->window(), SCR_WIDTH, SCR_HEIGHT);
    return true;
}

bool DynastyEngine::run() {
    initEngine();

    while (!glfwWindowShouldClose(editorUI_->window())) {
        glfwPollEvents();
        drawFrame();
        if (!configPanel_->initialize()) {
            configPanel_->initImgui(editorUI_->window(), viewer_->getRenderer());
        }
        // FIXME: 需要后面修复 
        drawPanel();
    }
}

void DynastyEngine::setupConfigPanelActions() {
    configPanel_->setReloadModelFunc([&](const std::string &path) -> bool {
      waitRenderIdle();
      return modelLoader_->loadModel(path);
    });
    // configPanel_->setReloadSkyboxFunc([&](const std::string &path) -> bool {
    //   waitRenderIdle();
    //   return modelLoader_->loadSkybox(path);
    // });
    // configPanel_->setFrameDumpFunc([&]() -> void {
    //   dumpFrame_ = true;
    // });
}

void DynastyEngine::cleanup() {
}