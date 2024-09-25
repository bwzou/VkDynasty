#pragma once
#include "runtime/framework/component/Component.h"

// #include <fmod.hpp>

namespace DynastyEngine 
{
    class SoundComponent : public Component
    {
    public:
        SoundComponent() = default;
        SoundComponent(const SoundComponent&) = default;
        SoundComponent(const std::string& path) : Component("SoundComponent") {
            mPath = path;
        }

        std::string mPath = "None";
        bool pPlay;

        // FMOD::Sound* Sound;
        // FMOD::Channel* Channel = nullptr;
    };
}