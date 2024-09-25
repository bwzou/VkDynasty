#pragma once

#include "runtime/framework/system/System.h"

namespace DynastyEngine
{
    class AudioScriptSystem : public System
    {
    public:
        AudioScriptSystem(Level* level);
        virtual ~AudioScriptSystem() = default;

    };
} 
