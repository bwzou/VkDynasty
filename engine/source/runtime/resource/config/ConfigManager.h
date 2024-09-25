#pragma once

#include <filesystem>

namespace DynastyEngine
{
    class ConfigManager
    {
    public:
        ConfigManager() {}
        ~ConfigManager() {}

        void initialize(const std::filesystem::path& config_file_path);

        const std::filesystem::path& getRootFolder() const;
        const std::filesystem::path& getAssetFolder() const;
        const std::filesystem::path& getSchemaFolder() const;
        // const std::filesystem::path& getEditorBigIconPath() const;
        // const std::filesystem::path& getEditorSmallIconPath() const;
        const std::filesystem::path& getEditorFontPath() const;
        const std::filesystem::path& getShaderGlslDir() const;
        const std::string& getGlobalRenderingResUrl() const;
        // const std::string& getGlobalParticleResUrl() const;

    private:
        std::filesystem::path mRootFolder = "/Users/bowenzou/Workspace/Documents/dynasty";
        std::filesystem::path mAssetFolder;
        std::filesystem::path mSchemaFolder;
        std::filesystem::path mEditorBigIconPath;
        std::filesystem::path mEditorSmallIconPath;
        std::filesystem::path mEditorFontPath;
        std::filesystem::path mShaderGlslDir = "/Users/bowenzou/Workspace/Documents/dynasty/engine/shader/generated/spv/";

        std::filesystem::path mGlobalRenderingRes = "/Users/bowenzou/Workspace/Documents/dynasty/engine/asset/global/rendering.global.json";
        // std::filesystem::path mGlobalRenderingRes = "/Users/bowenzou/Workspace/Documents/dynasty/engine/asset/global/rendering.global.json";
    };
}