#pragma once

#include "runtime/framework/component/Component.h"
#include "runtime/code/base/GLMInc.h"


namespace DynastyEngine
{
    class RenderCamera;

    enum class CameraMode : unsigned char
    {
        ThirdPerson = 0,
        TirstPerson,
        Free,
        Invalid
    };

    class CameraComponent : public Component
    {
    public:
        CameraComponent() = default;

        void tick(float deltaTime);

        

    private: 
        CameraMode mCameraMode {CameraMode::Invalid};

        glm::vec3 mPosition;
        glm::vec3 mForward {0, -1, 0};
        glm::vec3 mUp {0, 0, 1};
        glm::vec3 mLeft {1, 0, 0};
    };
}
