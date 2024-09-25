#pragma once
#include "runtime/code/base/GLMInc.h"


namespace DynastyEngine
{
    enum class EditorCommand : unsigned int
    {
        CameraLeft      = 1 << 0,  // A
        CameraBack      = 1 << 1,  // S
        CameraFoward    = 1 << 2,  // W
        CameraRight     = 1 << 3,  // D
        CameraUp        = 1 << 4,  // Q
        CameraDown      = 1 << 5,  // E
        TranslationMode = 1 << 6,  // T
        RotationMode    = 1 << 7,  // R
        ScaleMode       = 1 << 8,  // C
        Exit             = 1 << 9,  // Esc
        DeleteObject    = 1 << 10, // Delete
    };

    class EditorInputManager
    {
    public:
        void initialize();
        void tick(float deltaTime);

        void registerInputEvent();
        void processEditorCommand();
        void onKeyInEditorMode(int key, int scancode, int action, int mods);
        void onKey(int key, int scancode, int action, int mods);
        void onReset();
        void onCursorPos(double xPos, double yPos);
        void onCursorEnter(int entered);
        void onScroll(double xOffset, double yOffset);
        void onMouseButtonClicked(int key, int action);
        void onWindowClosed();

        bool isCursorInRect(glm::vec2 pos, glm::vec2 size) const;

        glm::vec2 getEngineWindowPos() const { return mEngineWindowPos; };
        glm::vec2 getEngineWindowSize() const { return mEngineWindowSize; };
        float     getCameraSpeed() const { return mCameraSpeed; }
        
        void setEngineWindowPos(glm::vec2 newWindowPos) { mEngineWindowPos = newWindowPos; };
        void setEngineWindowSize(glm::vec2 newWindowSize) { mEngineWindowSize = newWindowSize; };
        void resetEditorCommand() { mEditorCommand = 0; } 

    private:
        glm::vec2 mEngineWindowPos {0.0f, 0.0f};
        glm::vec2 mEngineWindowSize {1280.0f, 768.0f};
        float     mMouseX {0.0f};
        float     mMouseY {0.0f};
        float     mCameraSpeed {0.05f};

        size_t       mCursorOnAxis {3};
        unsigned int mEditorCommand {0};
    };
}