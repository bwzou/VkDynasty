#include "runtime/platform/WindowSystem.h"
#include "runtime/code/base/macro.h"


namespace DynastyEngine {
    WindowSystem::~WindowSystem()
    {
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    void WindowSystem::initialize(WindowCreateInfo createInfo) 
    {
        if (!glfwInit())
        {
            LOG_FATAL(__FUNCTION__, "failed to initialize GLFW");
            return;
        }

        mWidth  = createInfo.width;
        mHeight = createInfo.height;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        mWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, nullptr, nullptr);
        if (!mWindow)
        {
            LOG_FATAL(__FUNCTION__, "failed to create window");
            glfwTerminate();
            return;
        }

        // Setup input callbacks
        initEvent();
    }

    void WindowSystem::initEvent()
    {
        /* Make the window's context current */
        glfwSetWindowUserPointer(mWindow, this);
        glfwSetKeyCallback(mWindow, keyCallback);
        glfwSetCharCallback(mWindow, charCallback);
        glfwSetCharModsCallback(mWindow, charModsCallback);
        glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
        glfwSetCursorPosCallback(mWindow, cursorPosCallback);
        glfwSetCursorEnterCallback(mWindow, cursorEnterCallback);
        glfwSetScrollCallback(mWindow, scrollCallback);
        glfwSetDropCallback(mWindow, dropCallback);
        glfwSetWindowSizeCallback(mWindow, windowSizeCallback);
        glfwSetWindowCloseCallback(mWindow, windowCloseCallback);

        glfwSetInputMode(mWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    }

    void WindowSystem::pollEvents() const { glfwPollEvents(); }

    bool WindowSystem::shouldClose() const { return glfwWindowShouldClose(mWindow); }

    void WindowSystem::setTitle(const char* title) { glfwSetWindowTitle(mWindow, title); }

    GLFWwindow* WindowSystem::getWindow() const { return mWindow; }

    std::array<int, 2> WindowSystem::getWindowSize() const { return std::array<int, 2>({mWidth, mHeight}); }

    void WindowSystem::setFocusMode(bool mode)
    {
        pIsFocusMode = mode;
        glfwSetInputMode(mWindow, GLFW_CURSOR, pIsFocusMode ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}