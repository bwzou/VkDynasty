#include "DynastyEngine.h"


extern std::shared_ptr<DynastyEngine::DEngine> app;
namespace DynastyEngine
{
    double lastX = SCR_WIDTH; // 因为屏幕是2000 1600, 所以这里不需要除以2
    double lastY = SCR_HEIGHT;
    bool firstMouse = true;

    void framebufferSizeCallback(GLFWwindow *window, int width, int height)
    {
        LOG_INFO("=== Do nothing! framebufferSizeCallback ===");
    }


    void mouseCallback(GLFWwindow *window, double xPos, double yPos) 
    {
        LOG_INFO("=== Do nothing! mouseCallback ===");

        if (!app || app->wantCaptureMouse()) 
        {
            return;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) 
        {
            if (firstMouse) 
            {
                lastX = xPos;
                lastY = yPos;
                firstMouse = false;
            }
            double xOffset = xPos - lastX;
            double yOffset = yPos - lastY;

            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) 
            {
                app->updateGesturePan((float) xOffset, (float)yOffset);
            } 
            else 
            {
                app->updateGestureRotate((float) xOffset, (float) yOffset);
            }
            lastX = xPos;
            lastY = yPos;

        } 
        else 
        {
            firstMouse = true;
        }
    }


    void scrollCallback(GLFWwindow *window, double xOffset, double yOffset) 
    {
        LOG_INFO("=== Do nothing! scrollCallback ===");

        if (!app || app->wantCaptureMouse()) 
        {
            return;
        }
        app->updateGestureZoom((float) xOffset, (float) yOffset);
    }


    void processInput(GLFWwindow *window) 
    {
        LOG_INFO("=== Do nothing! processInput ===");

        if (!app || app->wantCaptureKeyboard()) 
        {
            return;
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
        {
            glfwSetWindowShouldClose(window, true);
            return;
        }

        static bool keyPressed_H = false;
        int state = glfwGetKey(window, GLFW_KEY_H);
        if (state == GLFW_PRESS) 
        {
            if (!keyPressed_H) 
            {
                keyPressed_H = true;
                app -> togglePanelState();
            }
        } 
        else if (state == GLFW_RELEASE) 
        {
            keyPressed_H = false;
        }
    }


    void DEngine::drawFrame() 
    {
        orbitController_->update();
        camera_->update();
        configPanel_->update();

        // update triangle count
        config_->triangleCount_ = modelLoader_->getModelPrimitiveCnt();
        
        // viewer_->create(editorUI_->window(), SCR_WIDTH, SCR_HEIGHT, NULL);
        
        viewer_->configRenderer();
        
        viewer_->drawFrame(modelLoader_->getScene());

        // viewer_->swapBuffer();
    }


    bool DEngine::initEngine() 
    {
        // camera
        camera_ = std::make_shared<Camera>();
        camera_->setPerspective(glm::radians(CAMERA_FOV), (float) SCR_WIDTH / (float) SCR_HEIGHT, CAMERA_NEAR, CAMERA_FAR);

        // orbit controller
        orbitController_ = std::make_shared<SmoothOrbitController>(std::make_shared<OrbitController>(*camera_));

        config_ = std::make_shared<Config>();
        configPanel_ = std::make_shared<ConfigPanel>(*config_);

        editorUI_ = std::make_shared<DynastyEditorUI>(*config_);
        editorUI_->initWindow();
        initEvent(editorUI_->getWindow());
        
        viewer_ = std::make_shared<DynastyViewer>(*config_, *camera_);
        viewer_->create(editorUI_->getWindow(), SCR_WIDTH, SCR_HEIGHT, NULL);
            
        modelLoader_ = std::make_shared<ModelLoader>(*config_);
        // setup config panel actions callback
        setupConfigPanelActions();

        // init config
        configPanel_->init(editorUI_->getWindow(), SCR_WIDTH, SCR_HEIGHT);
        return true;
    }


    bool DEngine::initEvent(GLFWwindow* window_) 
    {
        /* Make the window's context current */
        glfwMakeContextCurrent(window_);
        glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
        glfwSetCursorPosCallback(window_, mouseCallback);
        glfwSetScrollCallback(window_, scrollCallback);
    }


    bool DEngine::run() 
    {
        initEngine();

        while (!glfwWindowShouldClose(editorUI_->getWindow())) 
        {
            glfwPollEvents();
            drawFrame();
            if (!configPanel_->initialize()) 
            {
                configPanel_->initImgui(editorUI_->getWindow(), viewer_->getRenderer());
            }
            // FIXME: 需要后面修复 
            drawPanel();
        }
    }


    void DEngine::setupConfigPanelActions() 
    {
        configPanel_->setReloadModelFunc([&](const std::string &path) -> bool 
        {
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
        configPanel_->setUpdateLightFunc([&](glm::vec3 &position, glm::vec3 &color) -> void 
        {
            auto &scene = modelLoader_->getScene();
            scene.pointLight.vertexes[0].a_position = position;
            scene.pointLight.UpdateVertexes();
            scene.pointLight.material->baseColor = glm::vec4(color, 1.f);
        });
    }


    void DEngine::cleanup() 
    {

    }
}