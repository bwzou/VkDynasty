#include "editor/managers/EditorInputManager.h"
#include "editor/managers/EditorSceneManager.h"
#include "editor/global/GlobalContext.h"

#include "runtime/global/GlobalContext.h"
#include "runtime/platform/WindowSystem.h"
#include "runtime/render/RenderSystem.h"
#include "runtime//framework/level/level.h"


namespace DynastyEngine 
{
    unsigned int k_complement_control_command = 0xFFFFFFFF;

    void EditorInputManager::initialize()
    {
        registerInputEvent();
    }
    
    void EditorInputManager::tick(float deltaTime)
    {
        processEditorCommand();
    }

    void EditorInputManager::registerInputEvent()
    {
        gEditorGlobalContext.mWindowSystem->registerOnKeyFunc(std::bind(&EditorInputManager::onKey, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        gEditorGlobalContext.mWindowSystem->registerOnResetFunc(std::bind(&EditorInputManager::onReset, this));
        gEditorGlobalContext.mWindowSystem->registerOnCursorPosFunc(std::bind(&EditorInputManager::onCursorPos, this, std::placeholders::_1, std::placeholders::_2));
        gEditorGlobalContext.mWindowSystem->registerOnCursorEnterFunc(std::bind(&EditorInputManager::onCursorEnter, this, std::placeholders::_1));
        gEditorGlobalContext.mWindowSystem->registerOnScrollFunc(std::bind(&EditorInputManager::onScroll, this, std::placeholders::_1, std::placeholders::_2));
        gEditorGlobalContext.mWindowSystem->registerOnMouseButtonFunc(std::bind(&EditorInputManager::onMouseButtonClicked, this, std::placeholders::_1, std::placeholders::_2));
        gEditorGlobalContext.mWindowSystem->registerOnWindowCloseFunc(std::bind(&EditorInputManager::onWindowClosed, this));  
    }

    void EditorInputManager::processEditorCommand()
    {
        float           cameraSpeed  = mCameraSpeed;
        std::shared_ptr editorCamera = gEditorGlobalContext.mSceneManager->getEditorCamera();
        Quaternion      cameraRotate = editorCamera->rotation().inverse();
        glm::vec3       cameraRelativePos(0, 0, 0);

        if ((unsigned int)EditorCommand::CameraFoward & mEditorCommand)
        {
            cameraRelativePos += cameraRotate * glm::vec3 {0, cameraSpeed, 0};
        }
        if ((unsigned int)EditorCommand::CameraBack & mEditorCommand)
        {
            cameraRelativePos += cameraRotate * glm::vec3 {0, -cameraSpeed, 0};
        }
        if ((unsigned int)EditorCommand::CameraLeft & mEditorCommand)
        {
            cameraRelativePos += cameraRotate * glm::vec3 {-cameraSpeed, 0, 0};
        }
        if ((unsigned int)EditorCommand::CameraRight & mEditorCommand)
        {
            cameraRelativePos += cameraRotate * glm::vec3 {cameraSpeed, 0, 0};
        }
        if ((unsigned int)EditorCommand::CameraUp & mEditorCommand)
        {
            cameraRelativePos += glm::vec3 {0, 0, cameraSpeed};
        }
        if ((unsigned int)EditorCommand::CameraDown & mEditorCommand)
        {
            cameraRelativePos += glm::vec3 {0, 0, -cameraSpeed};
        }
        if ((unsigned int)EditorCommand::DeleteObject & mEditorCommand)
        {
            // gEditorGlobalContext.mSceneManager->onDeleteSelectedGObject();
        }

        editorCamera->move(cameraRelativePos);
    }

    void EditorInputManager::onKeyInEditorMode(int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
        {
            switch (key)
            {
                case GLFW_KEY_A:
                    mEditorCommand |= (unsigned int)EditorCommand::CameraLeft;
                    break;
                case GLFW_KEY_S:
                    mEditorCommand |= (unsigned int)EditorCommand::CameraBack;
                    break;
                case GLFW_KEY_W:
                    mEditorCommand |= (unsigned int)EditorCommand::CameraFoward;
                    break;
                case GLFW_KEY_D:
                    mEditorCommand |= (unsigned int)EditorCommand::CameraRight;
                    break;
                case GLFW_KEY_Q:
                    mEditorCommand |= (unsigned int)EditorCommand::CameraUp;
                    break;
                case GLFW_KEY_E:
                    mEditorCommand |= (unsigned int)EditorCommand::CameraDown;
                    break;
                case GLFW_KEY_T:
                    mEditorCommand |= (unsigned int)EditorCommand::TranslationMode;
                    break;
                case GLFW_KEY_R:
                    mEditorCommand |= (unsigned int)EditorCommand::RotationMode;
                    break;
                case GLFW_KEY_C:
                    mEditorCommand |= (unsigned int)EditorCommand::ScaleMode;
                    break;
                case GLFW_KEY_DELETE:
                    mEditorCommand |= (unsigned int)EditorCommand::DeleteObject;
                    break;
                default:
                    break;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            switch (key)
            {
                case GLFW_KEY_ESCAPE:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::Exit);
                    break;
                case GLFW_KEY_A:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::CameraLeft);
                    break;
                case GLFW_KEY_S:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::CameraBack);
                    break;
                case GLFW_KEY_W:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::CameraFoward);
                    break;
                case GLFW_KEY_D:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::CameraRight);
                    break;
                case GLFW_KEY_Q:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::CameraUp);
                    break;
                case GLFW_KEY_E:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::CameraDown);
                    break;
                case GLFW_KEY_T:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::TranslationMode);
                    break;
                case GLFW_KEY_R:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::RotationMode);
                    break;
                case GLFW_KEY_C:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::ScaleMode);
                    break;
                case GLFW_KEY_DELETE:
                    mEditorCommand &= (k_complement_control_command ^ (unsigned int)EditorCommand::DeleteObject);
                    break;
                default:
                    break;
            }
        }
    }
    
    void EditorInputManager::onKey(int key, int scancode, int action, int mods)
    {
        onKeyInEditorMode(key, scancode, action, mods);
    }

    void EditorInputManager::onReset()
    {

    }
        
    void EditorInputManager::onCursorPos(double xPos, double yPos)
    {
        float angularVelocity = 180.0f / Math::max(mEngineWindowSize.x, mEngineWindowSize.y); // 180 degrees while moving full screen
        if (mMouseX >= 0.0f && mMouseY >= 0.0f)
        {
            if (gEditorGlobalContext.mWindowSystem->isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
            {
                LOG_INFO("GLFW_MOUSE_BUTTON_RIGHT {}", GLFW_MOUSE_BUTTON_RIGHT);
                glfwSetInputMode(gEditorGlobalContext.mWindowSystem->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                gEditorGlobalContext.mSceneManager->getEditorCamera()->rotate(glm::vec2(yPos - mMouseY, xPos - mMouseX) * angularVelocity);
            }
            else if (gEditorGlobalContext.mWindowSystem->isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
            {
                // gEditorGlobalContext.mSceneManager->moveEntity(
                //     xPos,
                //     yPos,
                //     mMouseX,
                //     mMouseY,
                //     mEngineWindowPos,
                //     mEngineWindowSize,
                //     mCursorOnAxis,
                //     gEditorGlobalContext.mSceneManager->getSelectedObjectMatrix()
                // );
                glfwSetInputMode(gEditorGlobalContext.mWindowSystem->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else
            {
                glfwSetInputMode(gEditorGlobalContext.mWindowSystem->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                if (isCursorInRect(mEngineWindowPos, mEngineWindowSize))
                {
                    glm::vec2 cursorUv = glm::vec2((mMouseX - mEngineWindowPos.x) / mEngineWindowSize.x, 
                                                    (mMouseY - mEngineWindowPos.y) / mEngineWindowSize.y);
                    // updateCursorOnAxis(cursorUv);
                }
            }
        }

        mMouseX = xPos;
        mMouseY = yPos;
    }
    
    void EditorInputManager::onCursorEnter(int entered)
    {
        if (!entered) // lost focus
        {
            mMouseX = mMouseY = -1.0f;
        }
    }
    
    void EditorInputManager::onScroll(double xOffset, double yOffset)
    {
        if (isCursorInRect(mEngineWindowPos, mEngineWindowSize))
        {
            if (gEditorGlobalContext.mWindowSystem->isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
            {
                if (yOffset > 0) 
                {
                    mCameraSpeed *= 1.2f;
                }
                else 
                {
                    mCameraSpeed *= 0.8f;
                }
            } 
            else 
            {  
                gEditorGlobalContext.mSceneManager->getEditorCamera()->zoom(
                    (float)yOffset * 2.0f
                );
            }
        }
    }
    
    void EditorInputManager::onMouseButtonClicked(int key, int action)
    {
        if (mCursorOnAxis != 3)
            return;
        
        // std::shared_ptr<Level> currentActiveLevel = gRuntimeGlobalContext.mWorldManager->getCurrentActiveLevel();
        // if (currentActiveLevel == nullptr)
        //     return;
        
        if (isCursorInRect(mEngineWindowPos, mEngineWindowSize))
        {
            if (key == GLFW_MOUSE_BUTTON_LEFT)
            {
                glm::vec2 pickUp((mMouseX - mEngineWindowPos.x) / mEngineWindowSize.x,
                                 (mMouseY - mEngineWindowPos.y) / mEngineWindowSize.y);
                // size_t selectMeshId = gEditorGlobalContext.mSceneManager->getGuidOfPickedMesh(pickUp);

                // size_t gobjectId = gEditorGlobalContext.mRenderSystem->getGObjectIDByMeshID(selectMeshId);
                // gRuntimeGlobalContext.mSceneManager->onGObjectSelected(gObjectId);
            }
        }
    }
    
    void EditorInputManager::onWindowClosed()
    {
        
    }

    bool EditorInputManager::isCursorInRect(glm::vec2 pos, glm::vec2 size) const
    {
        return pos.x <= mMouseX && mMouseX <= pos.x + size.x && pos.y <= mMouseY && mMouseY <= pos.y + size.y;
    }
}