#include "runtime/framework/level/Level.h"
#include "runtime/framework/entity/Entity.h"
#include "runtime/code/base/UUID.h"

#include "runtime/framework/system/script/LuaScriptSystem.h"
#include "runtime/framework/system/script/NativeScriptSystem.h"
#include "runtime/framework/system/script/PythonScriptSystem.h"

namespace DynastyEngine 
{
    Level::Level()
    {
        mSystems.emplace_back( std::make_shared<NativeScriptSystem>(this) );
        mSystems.emplace_back( std::make_shared<LuaScriptSystem>(this) );
        mSystems.emplace_back( std::make_shared<PythonScriptSystem>(this) );
        
    }

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
        for (auto& system : mSystems)
		{
			system->tick(deltaTime);
		}
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