#include "runtime/framework/level/Level.h"
#include "runtime/framework/entity/Entity.h"
#include "runtime/code/base/UUID.h"


namespace DynastyEngine 
{
    // bool Level::load(const std::string& levelResourceUrl) 
    // {

    // }
    
    // void Level::unload()
    // {

    // }

    // bool Level::save() 
    // {

    // }

    void Level::tick(float deltaTime)
    {

    }

    Entity Level::createEntity(const std::string& name)
    {
        return createEntityWithUUID(UUID<Entity>(), name);
    }

    Entity Level::createEntityWithUUID(UUID<Entity> uuid, const std::string& name)
    {

    }

    void Level::destroyEntity()
    {
        
    }

    void Level::duplicateEntity(Entity entity) 
    {   

    }
    
    
}