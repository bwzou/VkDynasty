#pragma once 

#include "runtime/framework/system/System.h"

namespace DynastyEngine
{
    class LuaScriptSystem : public System
    {
    public:
        LuaScriptSystem(Level* level);
        virtual ~LuaScriptSystem() = default;

    };
}