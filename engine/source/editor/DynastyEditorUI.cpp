#include "DynastyEditorUI.h"


namespace DynastyEngine
{
    static bool bShowEngineSettings = true;
	static bool bShowSceneSettings = true;
    static bool bShowContentBrowser = true;
	static bool bShowSceneHierachy = true;

    int DynastyEditorUI::initWindow() 
    {
        /* Initialize the library */
        glfwSetErrorCallback(glfwErrorCallback);
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
            return -1;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        /* create a windowed mode window and its OpenGL context */
        mWindow = glfwCreateWindow(WIDTH, HEIGHT, "MiniRenderer", nullptr, nullptr);
        if (!mWindow) {
            throw std::runtime_error("Failed to create GLFW window");
            glfwTerminate();
            return -1;
        }

        return 1;
    }


    void DynastyEditorUI::onDraw() 
    {
        
    }


    void DynastyEditorUI::cleanup() 
    {
        glfwDestroyWindow(mWindow);

        glfwTerminate();
    }
}

