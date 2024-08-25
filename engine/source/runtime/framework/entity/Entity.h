
#include "runtime/code/base/UUID.h"
#include "runtime/framework/component/Component.h"
#include <vector>


namespace DynastyEngine 
{
    class Entity
    {
    public:
        Entity() = default;
        // Entity(UUID<Entity> uuid, Level* level);
        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& addComponent(Args&&... args)
        {

        }

        template<typename T>
        T& getComponent()
        {

        }

        template<typename T>
        bool hasComponent(const std::string& compenentTypeName) const
        {
            // const std::string& = 
            for (const auto& component : mComponents)
            {
                if (component.getTypeName() == compenentTypeName)
                    return true;
            }

            return false;
        }

        int getUUID();

    private:
        std::vector<Component> mComponents;
    };
}