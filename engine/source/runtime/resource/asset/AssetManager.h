#pragma once

#include "runtime/code/base/macro.h"
#include "runtime/code/base/Json.h"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cassert>


namespace DynastyEngine 
{
    class AssetManager 
    {
    public:
        template<typename AssetType> 
        bool loadAsset(const std::string& assetUrl, AssetType& outAsset) const
        {
            // read json file to string
            std::filesystem::path assetPath = getFullPath(assetUrl);
            std::ifstream assetJsonFile(assetUrl);
            if (!assetJsonFile) 
            {
                LOG_ERROR("open file: {} failed!", assetPath.generic_string());
                return false;
            }

            std::stringstream buffer;
            buffer << assetJsonFile.rdbuf();
            std::string assetJsonText(buffer.str());

            // parse to json object and read to runtime res object
            std::string error;
            auto&& assetJson = Json::parse(assetJsonText, error);
            if (!error.empty())
            {
                LOG_ERROR("parse json file {} failed!", assetUrl);
                return false;
            }

            read(assetJson, outAsset);
            return true;
        }

        template<typename AssetType>
        bool saveAsset(const AssetType& outAsset, const std::string& assetUrl) 
        {
            std::ofstream assetJsonFile(getFullPath(assetUrl));
            if (!assetJsonFile)
            {
                LOG_ERROR("open file {} failed!", assetUrl);
                return false;
            }

            // write to json object and dump to string
            auto&&  assetJson = write(outAsset);
            std::string&& assertJsonText = assetJson.dump();

            // write to file
            assetJsonFile << assertJsonText;
            assetJsonFile.flush();

            return true;
        }

        template<typename T>
        static Json write(const T& instance)
        {
            if constexpr (std::is_pointer<T>::value)
            {
                return writePointer((T)instance);
            }
            else
            {
                // static_assert(always_false<T>, "Serializer::write<T> has not been implemented yet!");
                return Json();
            }
        }

        template<typename T>
        static T& read(const Json& json_context, T& instance)
        {
            if constexpr (std::is_pointer<T>::value)
            {
                return readPointer(json_context, instance);
            }
            else
            {
                // static_assert(always_false<T>, "Serializer::read<T> has not been implemented yet!");
                return instance;
            }
        }

        template<typename T>
        static Json writePointer(T* instance)
        {
            return Json::object {{"$typeName", Json {"*"}}, {"$context", write(*instance)}};
        }

        template<typename T>
        static T*& readPointer(const Json& json_context, T*& instance)
        {
            assert(instance == nullptr);
            std::string type_name = json_context["$typeName"].string_value();
            assert(!type_name.empty());
            if ('*' == type_name[0])
            {
                instance = new T;
                read(json_context["$context"], *instance);
            }
            else
            {
                // instance = static_cast<T*>(
                //     Reflection::TypeMeta::newFromNameAndJson(type_name, json_context["$context"]).m_instance);
            }
            return instance;
        }

        std::filesystem::path getFullPath(const std::string& relativePath) const;
    };
}