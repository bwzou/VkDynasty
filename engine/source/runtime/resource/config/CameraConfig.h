#pragma once

#include "runtime/code/base/GLMInc.h"

namespace DynastyEngine
{
    class CameraPose
    {
    public:
        glm::vec3 mPosition {80.0, 80.0, 80.0};
        glm::vec3 mTarget {0.0, 0.0, 0.0};
        glm::vec3 mUp {0.0, 0.0, 1.0};
    };
    
    class CameraConfig
    {
    public:    
        CameraPose mPose;
        glm::vec2  mAspect {1280.0, 768.0};
        float      mZfar{1000.f};
        float      mZnear{0.01f};
    };
}

