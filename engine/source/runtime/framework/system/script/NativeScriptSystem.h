#pragma once 

#include "runtime/framework/system/System.h"

namespace DynastyEngine
{
    class NativeScriptSystem : public System
    {
    public:
        NativeScriptSystem(Level* level);
        virtual ~NativeScriptSystem() = default;
    };
}