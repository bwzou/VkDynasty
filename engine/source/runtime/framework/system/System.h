#include "runtime/framework/level/level.h"

namespace DynastyEngine 
{
    class System
    {
    public:
        System() = delete;
        System(Level* level) : mLevel(level) {}
        virtual ~System() = default;
    
    public:
        virtual void tick(float deltaTime) {}

    protected:
        Level* mLevel = nullptr;
    };
}