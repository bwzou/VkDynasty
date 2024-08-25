#pragma once

#include "runtime/framework/serializer/Serializer.h"
#include "runtime/framework/level/Level.h"


namespace DynastyEngine 
{
    class HeSerializer : public Serializer
    {
    public:
        HeSerializer(const std::shared_ptr<Level>& mLevel) : Serializer(mLevel) 
        {

        }
        ~HeSerializer()
        {
            
        }

        // bool serialize(const std::string& filepath) override;
        // bool serializeRuntime(const std::string& filepath) override;

        // bool deserialize(const std::string& filepath) override;
        // bool deserializeRuntime(const std::string& filepath) override;
    };
}