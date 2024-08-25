#pragma once

#include "runtime/framework/serializer/Serializer.h"
#include "runtime/framework/level/Level.h"


namespace DynastyEngine 
{
    class JsonSerializer : public Serializer
    {
    public:
        JsonSerializer(const std::shared_ptr<Level>& mLevel) : Serializer(mLevel) 
        {

        }
        ~JsonSerializer()
        {
            
        }

        bool serialize(const std::string& filepath) override;
        bool serializeRuntime(const std::string& filepath) override;

        bool deserialize(const std::string& filepath) override;
        bool deserializeRuntime(const std::string& filepath) override;
    };
}