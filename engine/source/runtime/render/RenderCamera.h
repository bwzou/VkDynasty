#pragma once

#include "runtime/function/Geometry.h"

constexpr float CAMERA_FOV = 60.f;
constexpr float CAMERA_NEAR = 0.01f;
constexpr float CAMERA_FAR = 100.f;

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

namespace DynastyEngine
{
    class RenderCamera
    {
    public:
        void update();
        
        inline void setReverseZ(bool reverse) 
        {
            pReverseZ = reverse;
        }

        void setPerspective(float fov, float aspect, float near, float far);

        void lookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up);

        glm::mat4 projectionMatrix() const;

        glm::mat4 viewMatrix() const;

        glm::vec3 getWorldPositionFromView(glm::vec3 pos) const;

        inline const Frustum &getFrustum() const { return mFrustum; }
        
        inline const float &fov() const { return mFov; }
        inline const float &aspect() const { return mAspect; }
        inline const float &near() const { return mNear; }
        inline const float &far() const { return mFar; }

        inline const glm::vec3 &eye() const { return mEye; }
        inline const glm::vec3 &center() const { return mCenter; }
        inline const glm::vec3 &up() const { return mUp; }

    private:
        bool pReverseZ = false;

        float mFov = glm::radians(60.f);
        float mAspect = 1.0f;
        float mNear = 0.01f;
        float mFar = 100.f;

        glm::vec3 mEye{};
        glm::vec3 mCenter{};
        glm::vec3 mUp{};

        Frustum mFrustum;
    };
}