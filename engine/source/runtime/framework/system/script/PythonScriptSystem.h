#pragma once 
#include "runtime/framework/system/System.h"

namespace DynastyEngine
{
    class PythonScriptSystem : public System
    {
    public:
        PythonScriptSystem(Level* level);
        virtual ~PythonScriptSystem() = default;

    };
}