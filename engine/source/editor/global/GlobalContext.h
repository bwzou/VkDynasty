#pragma once

namespace DynastyEngine 
{
    struct EditorGlobalContextInitInfo
    {
        class WindowSystem*   windowSystem;
        class RenderSystem*   renderSystem;
        class Engine*         engineRuntime;
    };
    
    class EditorGlobalContext
    {
    public:
        void initialize(const EditorGlobalContextInitInfo& initInfo);
        void startEditor();
        void shutdownEditor();
        void clear();

    public:
        class EditorSceneManager* mSceneManager {nullptr};
        class EditorInputManager* mInputManager {nullptr};
        class RenderSystem*       mRenderSystem {nullptr};
        class WindowSystem*       mWindowSystem {nullptr};
        class Engine*             mEngineRuntime {nullptr};

    };

    extern EditorGlobalContext gEditorGlobalContext;
}