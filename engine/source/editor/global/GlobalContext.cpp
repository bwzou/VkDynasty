#include "editor/global/GlobalContext.h"

#include "editor/managers/EditorSceneManager.h"
#include "editor/managers/EditorInputManager.h"
#include "runtime/platform/WindowSystem.h"
#include "runtime/render/RenderSystem.h"

namespace DynastyEngine 
{
    EditorGlobalContext gEditorGlobalContext;

    void EditorGlobalContext::initialize(const EditorGlobalContextInitInfo& initInfo)
    {   
        gEditorGlobalContext.mRenderSystem  = initInfo.renderSystem;
        gEditorGlobalContext.mWindowSystem  = initInfo.windowSystem;
        gEditorGlobalContext.mEngineRuntime = initInfo.engineRuntime;

        mSceneManager = new EditorSceneManager();
        mSceneManager->initialize();
        mInputManager = new EditorInputManager();
        mInputManager->initialize();
    }

    void EditorGlobalContext::startEditor()
    {

    }
    
    void EditorGlobalContext::shutdownEditor()
    {

    }
    
    void EditorGlobalContext::clear()
    {
        delete (mSceneManager);
        delete (mInputManager);
    }
}