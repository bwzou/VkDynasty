#include <filesystem>
#include "runtime/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/resource/asset/AssetManager.h"


namespace DynastyEngine 
{
    std::filesystem::path AssetManager::getFullPath(const std::string& relativePath) const
    {
        return std::filesystem::absolute(gRuntimeGlobalContext.mConfigManager->getRootFolder() / relativePath);
    }
}