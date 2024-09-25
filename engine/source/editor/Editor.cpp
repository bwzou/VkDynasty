#include "editor/Editor.h"
#include "editor/EditorUI.h"
#include "runtime/Engine.h"
#include "runtime/global/GlobalContext.h"
#include "runtime/render/RenderSystem.h"

#include "editor/global/GlobalContext.h"
#include "editor/managers/EditorInputManager.h"
#include "editor/managers/EditorSceneManager.h"

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

        EditorGlobalContextInitInfo initInfo = {
            gRuntimeGlobalContext.mWindowSystem.get(),
            gRuntimeGlobalContext.mRenderSystem.get(),
            engineRuntime
        };
        gEditorGlobalContext.initialize(initInfo);
        gEditorGlobalContext.mSceneManager->setEditorCamera(
            gRuntimeGlobalContext.mRenderSystem->getRenderCamera()
        );

        mEditorUI      = std::make_shared<EditorUI>();
        WindowUIInitInfo uiInitInfo = {gRuntimeGlobalContext.mWindowSystem,
                                       gRuntimeGlobalContext.mRenderSystem};
        mEditorUI->initialize(uiInitInfo);
    }
    
    void Editor::clear() 
    {
        gEditorGlobalContext.clear();
    }

    void Editor::run() 
    {
        float deltaTime;
        while (true)
        {
            deltaTime = mEngineRuntime->calculateDeltaTime();
            gEditorGlobalContext.mSceneManager->tick(deltaTime);
            gEditorGlobalContext.mInputManager->tick(deltaTime);
            if (!mEngineRuntime->tickOneFrame(deltaTime))
                return;
        }
    }
}