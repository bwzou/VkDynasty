
#include "runtime/framework/serializer/JsonSerializer.h"
#include "runtime/code/base/macro.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/global/GlobalContext.h"


namespace DynastyEngine 
{
    bool JsonSerializer::serialize(const std::string& filepath) 
    {
        if (mLevel == nullptr)
        {
            LOG_ERROR("save level failed, no active level");
            return;
        }

        LOG_INFO("saving level: {}", filepath);

        // const size_t                    objectCout    = mLevel->mEntities.size();
        // std::vector<ObjectInstanceRes>& outputObjects = output_level_res.m_objects;
        // outputObjects.resize(objectCout);

        // size_t object_index = 0;
        // for (const auto& idObjectPair : mLevel->mEntities)
        // {
        //     if (idObjectPair.second)
        //     {
        //         idObjectPair.second->save(outputObjects[object_index]);
        //         ++object_index;
        //     }
        // }

        // const bool isSaveSuccess = gRuntimeGlobalContext.mAssetManager->saveAsset(output_level_res, mLevel->mLevelResourceUrl);

        // if (isSaveSuccess == false)
        // {
        //     LOG_ERROR("failed to save {}", mLevel->mLevelResourceUrl);
        // }
        // else
        // {
        //     LOG_INFO("level save succeed");
        // }

        // return isSaveSuccess;
        return false;
    }

    bool JsonSerializer::serializeRuntime(const std::string& filepath)
    {
    
    }

    bool JsonSerializer::deserialize(const std::string& filepath) 
    {
        LOG_INFO("loading level: {}", filepath);

        // m_level_res_url = level_res_url;

        // LevelRes   level_res;
        // const bool isLoadSuccess = gRuntimeGlobalContext.mAssetManager->loadAsset(filepath, level_res);
        // if (isLoadSuccess == false)
        // {
        //     return false;
        // }

        // // ASSERT(gRuntimeGlobalContext.m_physics_manager);
        // // m_physics_scene = gRuntimeGlobalContext.m_physics_manager->createPhysicsScene(level_res.m_gravity);
        // // ParticleEmitterIDAllocator::reset();

        // for (const ObjectInstanceRes& object_instance_res : level_res.m_objects)
        // {
        //     mLevel->createEntity(object_instance_res);
        // }

        // // create active character
        // for (const auto& objectPair : mLevel->mEntities)
        // {
        //     std::shared_ptr<Entity> object = objectPair.second;
        //     if (object == nullptr)
        //         continue;

        //     if (level_res.m_character_name == object->getName())
        //     {
        //         m_current_active_character = std::make_shared<Character>(object);
        //         break;
        //     }
        // }

        // mLevel->pIsLoaded = true;

        LOG_INFO("level load succeed");

        return true;
    }

    bool JsonSerializer::deserializeRuntime(const std::string& filepath)
    {

    }

    // template<>
    // Json JsonSerializer::write(const char& instance)
    // {
    //     return Json(instance);
    // }
    // template<>
    // char& JsonSerializer::read(const Json& json_context, char& instance)
    // {
    //     assert(json_context.is_number());
    //     return instance = json_context.number_value();
    // }

    // template<>
    // Json JsonSerializer::write(const int& instance)
    // {
    //     return Json(instance);
    // }
    // template<>
    // int& JsonSerializer::read(const Json& json_context, int& instance)
    // {
    //     assert(json_context.is_number());
    //     return instance = static_cast<int>(json_context.number_value());
    // }

    // template<>
    // Json JsonSerializer::write(const unsigned int& instance)
    // {
    //     return Json(static_cast<int>(instance));
    // }
    // template<>
    // unsigned int& JsonSerializer::read(const Json& json_context, unsigned int& instance)
    // {
    //     assert(json_context.is_number());
    //     return instance = static_cast<unsigned int>(json_context.number_value());
    // }

    // template<>
    // Json JsonSerializer::write(const float& instance)
    // {
    //     return Json(instance);
    // }
    // template<>
    // float& JsonSerializer::read(const Json& json_context, float& instance)
    // {
    //     assert(json_context.is_number());
    //     return instance = static_cast<float>(json_context.number_value());
    // }

    // template<>
    // Json JsonSerializer::write(const double& instance)
    // {
    //     return Json(instance);
    // }
    // template<>
    // double& JsonSerializer::read(const Json& json_context, double& instance)
    // {
    //     assert(json_context.is_number());
    //     return instance = static_cast<float>(json_context.number_value());
    // }

    // template<>
    // Json JsonSerializer::write(const bool& instance)
    // {
    //     return Json(instance);
    // }
    // template<>
    // bool& JsonSerializer::read(const Json& json_context, bool& instance)
    // {
    //     assert(json_context.is_bool());
    //     return instance = json_context.bool_value();
    // }

    // template<>
    // Json JsonSerializer::write(const std::string& instance)
    // {
    //     return Json(instance);
    // }
    // template<>
    // std::string& JsonSerializer::read(const Json& json_context, std::string& instance)
    // {
    //     assert(json_context.is_string());
    //     return instance = json_context.string_value();
    // }
}