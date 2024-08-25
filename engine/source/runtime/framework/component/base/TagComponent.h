#include "runtime/framework/component/Component.h"
#include<iostream>

namespace DynastyEngine 
{
    class TagComponent : public Component
    {
    public:
        TagComponent(const std::string& tag) : Component("TagComponent") 
        {
            mTag = tag;
        }
        ~TagComponent() {
            
        }
    
    public:
        /* data */
        std::string mTag;
    }; 
}