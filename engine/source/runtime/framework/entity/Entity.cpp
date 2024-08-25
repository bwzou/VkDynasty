#include "runtime/framework/entity/Entity.h"
#include "runtime/framework/component/base/IDComponent.h"


namespace DynastyEngine 
{

    int Entity::getUUID() { return getComponent<IDComponent>().mID.get(); }
    
}