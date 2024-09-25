#pragma once

#include "runtime/framework/system/System.h"

namespace DynastyEngine
{
    class PhysicsSystem : public System
    {
    public:
        PhysicsSystem(Level* level);
        virtual ~PhysicsSystem() = default;

    };
} 