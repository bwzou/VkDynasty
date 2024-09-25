#include "runtime/render/RenderCamera.h"
#include <algorithm>

namespace DynastyEngine
{

    void RenderCamera::setPerspective(float fov, float aspect, float near, float far) 
    {
        mFov = fov;
        mAspect = aspect;
        mNear = near;
        mFar = far;
    }

    // void RenderCamera::lookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up) 
    // {
    //     mEye = eye;
    //     mCenter = center;
    //     mUp = up;
    // } 

    // 投影矩阵
    // https://ogldev.org/www/tutorial12/tutorial12.html
    // TODO: 我还没搞懂
    glm::mat4 RenderCamera::projectionMatrix() const 
    {
        float tanHalfFovInverse = 1.f / std::tan((mFov * 0.5f));

        glm::mat4 projection(0.f);
        projection[0][0] = tanHalfFovInverse / mAspect;
        projection[1][1] = tanHalfFovInverse;

        if (pReverseZ) 
        {
            projection[2][2] = 0.f;
            projection[2][3] = -1.f;
            projection[3][2] = mNear;
        } 
        else 
        {
            projection[2][2] = -1.f;
            projection[2][3] = -1.f;
            projection[3][2] = -mNear;
        }

        return projection;
    }

    // 可参考 https://blog.csdn.net/weixin_44179561/article/details/124149297
    // glm库储存矩阵元素采用的是列优先的储存方式
    // 所以mat[ i ][ j ]表示的是第 i 列，第 j 行元素
    glm::mat4 RenderCamera::viewMatrix() const 
    {
        // glm::vec3 forward(glm::normalize(centerPosition - eyePosition));
        // glm::vec3 side(glm::normalize(cross(forward, upDir)));
        // glm::vec3 up(glm::cross(side, forward));

        // glm::mat4 view(1.f);

        // view[0][0] = side.x;
        // view[1][0] = side.y;
        // view[2][0] = side.z;
        // view[3][0] = -glm::dot(side, mEye);

        // view[0][1] = up.x;
        // view[1][1] = up.y;
        // view[2][1] = up.z;
        // view[3][1] = -glm::dot(up, mEye);

        // view[0][2] = -forward.x;
        // view[1][2] = -forward.y;
        // view[2][2] = -forward.z;
        // view[3][2] = glm::dot(forward, mEye);

        // return view;
    }

    glm::vec3 RenderCamera::getWorldPositionFromView(glm::vec3 pos) const 
    {
        glm::mat4 proj, view, projInv, viewInv;
        proj = projectionMatrix();
        view = viewMatrix();

        projInv = glm::inverse(proj);
        viewInv = glm::inverse(view);

        glm::vec4 pos_world = viewInv * projInv * glm::vec4(pos, 1);
        pos_world /= pos_world.w;

        return glm::vec3{pos_world};
    }

    void RenderCamera::update() 
    {
        glm::vec3 forward(glm::normalize(mCenter - mEye));
        glm::vec3 side(glm::normalize(cross(forward, mUp)));
        glm::vec3 up(glm::cross(side, forward));

        float nearHeightHalf = mNear * std::tan(mFov / 2.f);
        float farHeightHalf = mFar * std::tan(mFov / 2.f);
        float nearWidthHalf = nearHeightHalf * mAspect;
        float farWidthHalf = farHeightHalf * mAspect;

        // near plane
        glm::vec3 nearCenter = mEye + forward * mNear;
        glm::vec3 nearNormal = forward;
        mFrustum.planes[0].set(nearNormal, nearCenter);
            
        // far plane
        glm::vec3 farCenter = mEye + forward * mFar;
        glm::vec3 farNormal = -forward;
        mFrustum.planes[1].set(farNormal, farCenter);

        // top plane
        glm::vec3 topCenter = nearCenter + up * nearHeightHalf;
        glm::vec3 topNormal = glm::cross(glm::normalize(topCenter - mEye), side);
        mFrustum.planes[2].set(topNormal, topCenter);

        // bottom plane
        glm::vec3 bottomCenter = nearCenter - up * nearHeightHalf;
        glm::vec3 bottomNormal = glm::cross(side, glm::normalize(bottomCenter - mEye));
        mFrustum.planes[3].set(bottomNormal, bottomCenter);

        // left plane
        glm::vec3 leftCenter = nearCenter - side * nearWidthHalf;
        glm::vec3 leftNormal = glm::cross(glm::normalize(leftCenter - mEye), up);
        mFrustum.planes[4].set(leftNormal, leftCenter);

        // right plane
        glm::vec3 rightCenter = nearCenter + side * nearWidthHalf;
        glm::vec3 rightNormal = glm::cross(up, glm::normalize(rightCenter - mEye));
        mFrustum.planes[5].set(rightNormal, rightCenter);

        // 8 corners
        glm::vec3 nearTopLeft = nearCenter + up * nearHeightHalf - side * nearWidthHalf;
        glm::vec3 nearTopRight = nearCenter + up * nearHeightHalf + side * nearWidthHalf;
        glm::vec3 nearBottomLeft = nearCenter - up * nearHeightHalf - side * nearWidthHalf;
        glm::vec3 nearBottomRight = nearCenter - up * nearHeightHalf + side * nearWidthHalf;

        glm::vec3 farTopLeft = farCenter + up * farHeightHalf - side * farWidthHalf;
        glm::vec3 farTopRight = farCenter + up * farHeightHalf + side * farWidthHalf;
        glm::vec3 farBottomLeft = farCenter - up * farHeightHalf - side * farWidthHalf;
        glm::vec3 farBottomRight = farCenter - up * farHeightHalf + side * farWidthHalf;

        mFrustum.corners[0] = nearTopLeft;
        mFrustum.corners[1] = nearTopRight;
        mFrustum.corners[2] = nearBottomLeft;
        mFrustum.corners[3] = nearBottomRight;
        mFrustum.corners[4] = farTopLeft;
        mFrustum.corners[5] = farTopRight;
        mFrustum.corners[6] = farBottomLeft;
        mFrustum.corners[7] = farBottomRight;

        // bounding box
        mFrustum.bbox.min = glm::vec3(std::numeric_limits<float>::max());
        mFrustum.bbox.max = glm::vec3(std::numeric_limits<float>::min());
        for (auto &corner : mFrustum.corners) 
        {
            mFrustum.bbox.min.x = std::min(mFrustum.bbox.min.x, corner.x);
            mFrustum.bbox.min.y = std::min(mFrustum.bbox.min.y, corner.y);
            mFrustum.bbox.min.z = std::min(mFrustum.bbox.min.z, corner.z);

            mFrustum.bbox.max.x = std::max(mFrustum.bbox.max.x, corner.x);
            mFrustum.bbox.max.y = std::max(mFrustum.bbox.max.y, corner.y);
            mFrustum.bbox.max.z = std::max(mFrustum.bbox.max.z, corner.z);
        }
    }





    void RenderCamera::lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
    {
        mPosition = position;

        // model rotation
        // maps vectors to camera space (x, y, z)
        glm::vec3 forward = glm::normalize((target - position));
        mRotation      = getRotationTo(forward, Y);

        // correct the up vector
        // the cross product of non-orthogonal vectors is not normalized
        glm::vec3 right  = glm::normalize(cross(forward, glm::normalize(up)));
        glm::vec3 orthUp = cross(right, forward);

        Quaternion upRotation = getRotationTo(mRotation * orthUp, Z);

        mRotation = Quaternion(upRotation) * mRotation;

        // inverse of the model rotation
        // maps camera space vectors to model vectors
        mInvRotation = mRotation.conjugate();
    }

    void RenderCamera::setAspect(float aspect) 
    {
        mAspect = aspect;

        // tan(mFovx * 0.5) = aspect * tan(mFovy * 0.5)
        // mFovx = std::atan(aspect * std::tan(glm::radians(mFovy) * 0.5)) * 2.0f;

        mFovy = Radian(Math::atan(Math::tan(Radian(Degree(mFovx) * 0.5f)) / mAspect) * 2.0f).valueDegrees();
    }

    void RenderCamera::move(glm::vec3 delta) 
    {
        mPosition += delta;
    }
    
    void RenderCamera::rotate(glm::vec2 delta)
    {
        // rotation around x, y axis
        delta = glm::vec2(Radian(Degree(delta.x)).valueRadians(), Radian(Degree(delta.y)).valueRadians());

        // limit pitch
        float dot = glm::dot(mUpAxis, forward());
        if ((dot < -0.99f && delta.x > 0.0f) || // angle nearing 180 degrees
            (dot > 0.99f && delta.x < 0.0f))    // angle nearing 0 degrees
            delta.x = 0.0f;

        // pitch is relative to current sideways rotation
        // yaw happens independently
        // this prevents roll
        Quaternion pitch, yaw;
        pitch.fromAngleAxis(Radian(delta.x), X);
        yaw.fromAngleAxis(Radian(delta.y), Z);

        mRotation = pitch * mRotation * yaw;

        mInvRotation = mRotation.conjugate();
    }

    void RenderCamera::zoom(float offset)
    {
        mFovy = glm::clamp(mFovy - offset, MIN_FOV, MAX_FOV);
    }

    glm::mat4 RenderCamera::makeLookAtMatrix(const glm::vec3 eyePosition, const glm::vec3 centerPosition, const glm::vec3 upDir)
    {
        glm::vec3 forward(glm::normalize(centerPosition - eyePosition));
        glm::vec3 side(glm::normalize(cross(forward, upDir)));
        glm::vec3 up(glm::cross(side, forward));

        glm::mat4 view(1.f);

        view[0][0] = side.x;
        view[1][0] = side.y;
        view[2][0] = side.z;
        view[3][0] = -glm::dot(side, eyePosition);

        view[0][1] = up.x;
        view[1][1] = up.y;
        view[2][1] = up.z;
        view[3][1] = -glm::dot(up, eyePosition);

        view[0][2] = -forward.x;
        view[1][2] = -forward.y;
        view[2][2] = -forward.z;
        view[3][2] = glm::dot(forward, eyePosition);

        return view;
    }

    glm::mat4 RenderCamera::makePersProjMatrix(float fovy, float aspect, float znear, float zfar)
    {
        float tanHalfFovInverse = 1.f / std::tan((fovy * 0.5f));

        glm::mat4 projection(0.f);
        projection[0][0] = tanHalfFovInverse / aspect;
        projection[1][1] = tanHalfFovInverse;
        projection[2][2] = (zfar + znear) / (znear - zfar);
        projection[2][3] = -1.f;
        projection[3][2] = -(zfar * znear) / (zfar - znear);

        return projection;
    }

    glm::mat4 RenderCamera::getViewMatrix() 
    {
        glm::mat4 viewMatrix = glm::mat4(1.0f);
        switch (mCurrentCameraType)
        {   
            case RenderCameraType::Editor:
                viewMatrix = makeLookAtMatrix(position(), position() + forward(), up());
                break;
            case RenderCameraType::Motor:
                viewMatrix = mViewMatrices[MAIN_VIEW_MATRIX_INDEX];
                break;
            default:
                break;
        }
        return viewMatrix;
    }

    glm::mat4 RenderCamera::getPersProjMatrix()
    {
        glm::mat4 fixMatrix = glm::mat4(1.0f);
        glm::mat4 projMatrix = fixMatrix * makePersProjMatrix(glm::radians(mFovy), mAspect, mZnear, mZfar);
        projMatrix[1][1] *= -1;
        return projMatrix;
    }
}