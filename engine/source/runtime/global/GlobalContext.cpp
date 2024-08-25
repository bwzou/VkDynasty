#include "runtime/global/GlobalContext.h"
#include "runtime/code/log/LogSystem.h"
#include "runtime/platform/WindowSystem.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/render/RenderSystem.h"
#include "runtime/debug/DebugManager.h"


namespace DynastyEngine {
    RuntimeGlobalContext gRuntimeGlobalContext;

    void RuntimeGlobalContext::startSystems(const std::string& config_file_path)
    {
        mLoggerSystem = std::make_shared<LogSystem>();

        mWindowSystem = std::make_shared<WindowSystem>();
        WindowCreateInfo windowCreateInfo;
        mWindowSystem->initialize(windowCreateInfo);

        mAssetManager = std::make_shared<AssetManager>();

        mConfigManager = std::make_shared<ConfigManager>();

        mRenderSystem = std::make_shared<RenderSystem>();
        RenderSystemInitInfo renderInitInfo;
        renderInitInfo.windowSystem = mWindowSystem;
        mRenderSystem->initialize(renderInitInfo);

        mDebugManager = std::make_shared<DebugManager>();
        mDebugManager->initialize();
    }

    void RuntimeGlobalContext::shutdownSystems()
    {
        mDebugManager.reset();

        mRenderSystem->clear();
        mRenderSystem.reset();

        mLoggerSystem.reset();

        mWindowSystem.reset();

        mConfigManager.reset();

        mAssetManager.reset();
    }
}