#pragma once
#include "runtime/code/base/GLMInc.h"

#include "runtime/render/RenderCamera.h"

namespace DynastyEngine 
{
    class EditorSceneManager
    {
    public:
        void initialize();
        void tick(float deltaTime);

        size_t updateCursorOnAxis();

        void moveEntity();

        void uploadAxisResource();

        size_t getGuidOfpickedMesh(const glm::vec2 pickedUp) const;

        void setEditorCamera(std::shared_ptr<RenderCamera> camera) {mCamera = camera;};
        std::shared_ptr<RenderCamera> getEditorCamera() {return mCamera;};

    private:
        std::shared_ptr<RenderCamera> mCamera;
        
    };
}