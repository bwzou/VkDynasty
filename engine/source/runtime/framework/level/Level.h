#pragma once

#include "runtime/code/base/UUID.h"

#include <memory>
#include <string>
#include <unordered_map>


namespace DynastyEngine 
{
    class Character;
    class Entity;

    using LevelObjectsMap = std::unordered_map<std::string, std::shared_ptr<Entity>>;

    class Level 
    {
    public:
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

    public:
        bool        pIsLoaded {false};

        std::string mLevelResourceUrl;
        
        // all ga,e objects in this level, key: object id, value: object instance
        LevelObjectsMap mEntities;

        std::shared_ptr<Character> mCurrentActiveCharacter;
    };
}