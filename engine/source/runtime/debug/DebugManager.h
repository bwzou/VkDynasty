#pragma once

#include <stdexcept>


namespace DynastyEngine 
{
    class DebugManager
    {
    public:
        DebugManager() = default;
        ~DebugManager();

        void initialize();
        void destroy();
        void clear();

        void tick(float deltaTime);
        void draw(uint32_t currentSwapchainImageIndex);

    private:
        

    };
}