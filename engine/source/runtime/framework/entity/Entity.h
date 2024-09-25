
#include "runtime/code/base/UUID.h"
#include "runtime/framework/component/Component.h"
#include "runtime/framework/level/Level.h"
#include <vector>

#include <entt.hpp>

namespace DynastyEngine 
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Level* level);
        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& addComponent(Args&&... args)
        {

        }

        template<typename T>
        T& getComponent()
        {
            return mLevel->mRegistry.get<T>(mEntityHandle);
        }

        template<typename T>
        bool hasComponent()
        {
            return mLevel->mRegistry.all_of<T>(mEntityHandle);
        }

        int getUUID();

    private:
        entt::entity mEntityHandle{ entt::null };
        Level* mLevel = nullptr;
    };
}