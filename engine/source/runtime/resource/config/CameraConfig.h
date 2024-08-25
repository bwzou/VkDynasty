#pragma once

#include "runtime/code/base/GLMInc.h"

namespace DynastyEngine
{
    class CameraPose
    {
        glm::vec3 mPosition;
        glm::vec3 mTarget;
        glm::vec3 mUp;
    };
    
    class CameraConfig
    {
        CameraPose mPose;
        glm::vec2  mAspect;
        float      mZFar{0.01f};
        float      mZNear{100.f};
    };
}

