#pragma once

#include "runtime/code/base/UUID.h"
#include "runtime/code/base/UUID.h"
// #include "runtime/framework/system/System.h"

#include <entt.hpp>
#include <memory>
#include <string>
#include <unordered_map>


namespace DynastyEngine 
{
    class Character;
    class Entity;
    class System;
    
    using LevelObjectsMap = std::unordered_map<std::string, std::shared_ptr<Entity>>;

    class Level 
    {
    public:
        Level();
        virtual ~Level(){};

        // bool load(const std::string& levelResourceUrl);
        
        // void unload();

        // bool save();

        Entity createEntity(const std::string& name = std::string());

        Entity createEntityWithUUID(UUID<Entity> uuid, const std::string& name = std::string());

        void destroyEntity();

        void duplicateEntity(Entity entity);

        void tick(float deltaTime);

        const std::string& getLevelResourceUrl() const { return mLevelResourceUrl; }

        // void LevelObjectsMap& getAllGObjects() const {return mGobject; }

        template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return mRegistry.view<Components...>();
		}

    public:
        bool        pIsLoaded {false};

        std::string mLevelResourceUrl;
        
        // all ga,e objects in this level, key: object id, value: object instance
        LevelObjectsMap mEntities;

        std::shared_ptr<Character> mCurrentActiveCharacter;

        entt::registry mRegistry;
        std::vector<std::shared_ptr<System>> mSystems;

        friend class Entity;
        
    };
}