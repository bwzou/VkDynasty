#pragma once

#include "runtime/code/math/quaternion.h"
#include "runtime/code/base/macro.h"
#include "runtime/function/Geometry.h"



constexpr float CAMERA_FOV = 60.f;
constexpr float CAMERA_NEAR = 0.01f;
constexpr float CAMERA_FAR = 100.f;

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

namespace DynastyEngine
{
    enum class RenderCameraType : int
    {
        Editor,
        Motor
    };

    class RenderCamera
    {
    public:
        void update();
        
        inline void setReverseZ(bool reverse) 
        {
            pReverseZ = reverse;
        }

        void setPerspective(float fov, float aspect, float near, float far);

        // void lookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up);

        glm::mat4 projectionMatrix() const;

        glm::mat4 viewMatrix() const;

        glm::vec3 getWorldPositionFromView(glm::vec3 pos) const;

        inline const Frustum &getFrustum() const { return mFrustum; }
        
        inline const float &fov() const { return mFov; }
        inline const float &aspect() const { return mAspect; }
        inline const float &near() const { return mNear; }
        inline const float &far() const { return mFar; }
        inline const glm::vec3 &center() const { return mCenter; }
        // inline const glm::vec3 &up() const { return mUp; }

    private:    
        bool pReverseZ = false;

        float mFov = glm::radians(60.f);
        // float mAspect = 1.0f;
        float mNear = 0.01f;
        float mFar = 100.f;

        glm::vec3 mEye{};
        glm::vec3 mCenter{};
        glm::vec3 mUp{};

        Frustum mFrustum;




    public:
        void setAspect(float aspect);
        void setFovy(float fovy) { mFovy = fovy; }
        void setFovx(float fovx) { mFovx = fovx; }

        inline const glm::vec3 position() const { return mPosition; }
        inline const Quaternion rotation() const { return mRotation; }
        inline const glm::vec3 forward() const {return mInvRotation * Y;}
        inline const glm::vec3 up() const {return mInvRotation * Z;}
        inline const glm::vec3 right() const {return mInvRotation * X;}
        glm::vec2 getFOV() const { return {mFovx, mFovy}; }
        
        void lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);
        glm::mat4 makePersProjMatrix(float fovy, float aspect, float znear, float zfar);
        glm::mat4 makeLookAtMatrix(const glm::vec3 eyePosition, const glm::vec3 centerPosition, const glm::vec3 upDir);
        glm::mat4 getViewMatrix();
        glm::mat4 getPersProjMatrix();
        
        void move(glm::vec3 delta);
        void rotate(glm::vec2 delta);
        void zoom(float offset);

        Quaternion getRotationTo(const glm::vec3& src, const glm::vec3& dest, const glm::vec3& fallback_axis = glm::vec3(0.f))
        {
            // Based on Stan Melax's article in Game Programming Gems
            Quaternion q;
            // Copy, since cannot modify local
            glm::vec3 v0 = glm::normalize(src);
            glm::vec3 v1 = glm::normalize(dest);

            float d = glm::dot(v0, v1);
            // If dot == 1, vectors are the same
            if (d >= 1.0f)
            {
                return Quaternion::IDENTITY;
            }
            if (d < (1e-6f - 1.0f))
            {
                if (fallback_axis != glm::vec3(0.f))
                {
                    // rotate 180 degrees about the fall back axis
                    q.fromAngleAxis(Radian(Math_PI), fallback_axis);
                }
                else
                {
                    // Generate an axis
                    glm::vec3 axis = glm::cross(glm::vec3({1, 0, 0}), src);
                    if (glm::length(axis) < (1e-06 * 1e-06)) // pick another if collinear
                        axis =  glm::cross(glm::vec3({0, 1, 0}), src);
                    axis = glm::normalize(axis);
                    q.fromAngleAxis(Radian(Math_PI), axis);
                }
            }
            else
            {
                float s    = Math::sqrt((1 + d) * 2);
                float invs = 1 / s;

                glm::vec3 c = glm::cross(v0, v1);

                q.x = c.x * invs;
                q.y = c.y * invs;
                q.z = c.z * invs;
                q.w = s * 0.5f;
                q.normalise();
            }
            return q;
        }

    public:
        RenderCameraType mCurrentCameraType {RenderCameraType::Editor};
        static const glm::vec3 X, Y, Z;

        glm::vec3  mPosition {0.0f, 0.0f, 0.0f}; // eye
        Quaternion mRotation {Quaternion::IDENTITY};
        Quaternion mInvRotation {Quaternion::IDENTITY};
        float      mZnear {1000.f};
        float      mZfar {0.1f};
        glm::vec3  mUpAxis {Z}; // 使用Z为向上方向

        static constexpr float MIN_FOV {10.0f};
        static constexpr float MAX_FOV {89.0f};
        static constexpr int   MAIN_VIEW_MATRIX_INDEX {0};

        std::vector<glm::mat4> mViewMatrices;

    protected:
        float mAspect {0.f};
        float mFovx {Degree(89.f).valueDegrees()};
        float mFovy {0.f};
        
        std::mutex mViewMatrixMutex;
    };

    inline const glm::vec3 RenderCamera::X = {1.0f, 0.0f, 0.0f};
    inline const glm::vec3 RenderCamera::Y = {0.0f, 1.0f, 0.0f};
    inline const glm::vec3 RenderCamera::Z = {0.0f, 0.0f, 1.0f};
}