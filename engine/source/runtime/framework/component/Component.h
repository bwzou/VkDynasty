#pragma once

#include <string>

namespace DynastyEngine {
    class Component {
    public:
        Component(const std::string type) {
            mType = type;
        }
        virtual ~Component() {};

        virtual std::string getTypeName() const;
    
    private:
        std::string mType;
    };
}