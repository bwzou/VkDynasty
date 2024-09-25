#pragma once

#include <memory>
#include <string>

namespace DynastyEngine {
    class LogSystem;
    class WindowSystem;
    class AssetManager;
    class ConfigManager;
    class RenderSystem;
    class DebugManager;


    class RuntimeGlobalContext
    {
    public:
        // create all global systems and initialize these systems
        void startSystems(const std::string& config_file_path);
        // destroy all global systems
        void shutdownSystems();
    
    public:
        std::shared_ptr<LogSystem>         mLoggerSystem;
        std::shared_ptr<WindowSystem>      mWindowSystem;
        std::shared_ptr<AssetManager>      mAssetManager;
        std::shared_ptr<ConfigManager>     mConfigManager;
        std::shared_ptr<RenderSystem>      mRenderSystem;
        std::shared_ptr<DebugManager>      mDebugManager;
    };

    extern RuntimeGlobalContext gRuntimeGlobalContext;
}