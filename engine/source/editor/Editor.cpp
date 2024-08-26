#include "editor/Editor.h"
#include "editor/EditorUI.h"
#include "runtime/Engine.h"
#include "runtime/global/GlobalContext.h"

namespace DynastyEngine {

    Editor::Editor() 
    {

    }

    Editor::~Editor() 
    {

    }

    void Editor::initialize(Engine* engineRuntime)
    {
        mEngineRuntime = engineRuntime;
        mEditorUI      = std::make_shared<EditorUI>();

        WindowUIInitInfo uiInitInfo = {gRuntimeGlobalContext.mWindowSystem,
                                       gRuntimeGlobalContext.mRenderSystem};
        mEditorUI->initialize(uiInitInfo);
    }
    
    void Editor::clear() 
    {
        
    }

    void Editor::run() 
    {
        float deltaTime;
        while (true)
        {
            deltaTime = mEngineRuntime->calculateDeltaTime();
            if (!mEngineRuntime->tickOneFrame(deltaTime))
                return;
        }
    }
}