#pragma once

#include <iostream>
#include <stdlib.h>

namespace DynastyEngine
{
    class EditorUI;
    class Engine;
    
    class Editor {
        friend class EditorUI;

    public:
        Editor();
        virtual ~Editor();

        void initialize(Engine* engineRuntime);
        void clear();

        void run();

    protected:
        std::shared_ptr<EditorUI> mEditorUI;
        Engine* mEngineRuntime{ nullptr };
    };
}