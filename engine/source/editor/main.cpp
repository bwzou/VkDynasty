#include <stdlib.h>
#include "runtime/Engine.h"
#include "editor/Editor.h"


int main(int argc, char** argv) 
{
    // std::filesystem::path executable_path(argv[0]);
    // std::filesystem::path config_file_path = executable_path.parent_path() / "PiccoloEditor.ini";

    DynastyEngine::Engine* engine = new DynastyEngine::Engine();
    engine->startEngine();

    DynastyEngine::Editor* editor = new DynastyEngine::Editor();
    editor->initialize(engine);

    editor->run();

    editor->clear();
    engine->clear();
    engine->shutdownEngine();
    
    return EXIT_SUCCESS;
}