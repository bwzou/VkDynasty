#pragma once

#include<iostream>
#include"runtime/framework/level/level.h" 


namespace DynastyEngine 
{
    class Serializer
    {
    public:
        Serializer(const std::shared_ptr<Level>& level) {}
        ~Serializer() {}

        virtual bool serialize(const std::string& filepath) {}
        virtual bool serializeRuntime(const std::string& filepath) {}

        virtual bool deserialize(const std::string& filepath) {}
        virtual bool deserializeRuntime(const std::string& filepath) {}

    protected:
        std::shared_ptr<Level> mLevel;
    };
}