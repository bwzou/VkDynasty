#include "runtime/framework/component/Component.h"
#include "runtime/code/base/UUID.h"


namespace DynastyEngine 
{
    class IDComponent : public Component
    {
    public:
        IDComponent(const UUID<Component>& id) : Component("IDComponent") 
        {
            mID = id;
        }   
        ~IDComponent() {
            
        }

    public:
        /* data */
        UUID<Component> mID;
    };
}