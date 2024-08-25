
#include "runtime/engine.h"
#include "runtime/resource/config/ConfigManager.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace DynastyEngine 
{
    void ConfigManager::initialize(const std::filesystem::path& configFilePath) 
    {
        
    }

    const std::filesystem::path& ConfigManager::getRootFolder() const { return mRootFolder; }

    const std::filesystem::path& ConfigManager::getAssetFolder() const { return mAssetFolder; }

    const std::filesystem::path& ConfigManager::getSchemaFolder() const { return mSchemaFolder; }

    const std::filesystem::path& ConfigManager::getEditorFontPath() const { return mEditorFontPath; }

    const std::filesystem::path& ConfigManager::getShaderGlslDir() const { return mShaderGlslDir; }
}