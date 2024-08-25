
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <functional>
#include <vector>

namespace DynastyEngine {
    struct WindowCreateInfo
    {
        int         width {1280};
        int         height {720};
        const char* title {"Piccolo"};
        bool        is_fullscreen {false};
    };

    class WindowSystem
    {
    public:
        WindowSystem() = default;
        ~WindowSystem();
        void               initialize(WindowCreateInfo createInfo);
        void               initEvent(); 
        void               pollEvents() const;
        bool               shouldClose() const;
        void               setTitle(const char* title);
        GLFWwindow*        getWindow() const;
        std::array<int, 2> getWindowSize() const;
        
        typedef std::function<void()>                   onResetFunc;
        typedef std::function<void(int, int, int, int)> onKeyFunc;
        typedef std::function<void(unsigned int)>       onCharFunc;
        typedef std::function<void(int, unsigned int)>  onCharModsFunc;
        typedef std::function<void(int, int, int)>      onMouseButtonFunc;
        typedef std::function<void(double, double)>     onCursorPosFunc;
        typedef std::function<void(int)>                onCursorEnterFunc;
        typedef std::function<void(double, double)>     onScrollFunc;
        typedef std::function<void(int, const char**)>  onDropFunc;
        typedef std::function<void(int, int)>           onWindowSizeFunc;
        typedef std::function<void()>                   onWindowCloseFunc;

        void registerOnResetFunc(onResetFunc func) { mOnResetFunc.push_back(func); }
        void registerOnKeyFunc(onKeyFunc func) { mOnKeyFunc.push_back(func); }
        void registerOnCharFunc(onCharFunc func) { mOnCharFunc.push_back(func); }
        void registerOnCharModsFunc(onCharModsFunc func) { mOnCharModsFunc.push_back(func); }
        void registerOnMouseButtonFunc(onMouseButtonFunc func) { mOnMouseButtonFunc.push_back(func); }
        void registerOnCursorPosFunc(onCursorPosFunc func) { mOnCursorPosFunc.push_back(func); }
        void registerOnCursorEnterFunc(onCursorEnterFunc func) { mOnCursorEnterFunc.push_back(func); }
        void registerOnScrollFunc(onScrollFunc func) { mOnScrollFunc.push_back(func); }
        void registerOnDropFunc(onDropFunc func) { mOnDropFunc.push_back(func); }
        void registerOnWindowSizeFunc(onWindowSizeFunc func) { mOnWindowSizeFunc.push_back(func); }
        void registerOnWindowCloseFunc(onWindowCloseFunc func) { mOnWindowCloseFunc.push_back(func); }

        bool isMouseButtonDown(int button) const
        {
            if (button < GLFW_MOUSE_BUTTON_1 || button > GLFW_MOUSE_BUTTON_LAST)
            {
                return false;
            }
            return glfwGetMouseButton(mWindow, button) == GLFW_PRESS;
        }
        bool getFocusMode() const { return pIsFocusMode; }
        void setFocusMode(bool mode);

    protected:
        // window event callbacks
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onKey(key, scancode, action, mods);
            }
        }
        static void charCallback(GLFWwindow* window, unsigned int codepoint)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onChar(codepoint);
            }
        }
        static void charModsCallback(GLFWwindow* window, unsigned int codepoint, int mods)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onCharMods(codepoint, mods);
            }
        }
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onMouseButton(button, action, mods);
            }
        }
        static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onCursorPos(xpos, ypos);
            }
        }
        static void cursorEnterCallback(GLFWwindow* window, int entered)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onCursorEnter(entered);
            }
        }
        static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onScroll(xoffset, yoffset);
            }
        }
        static void dropCallback(GLFWwindow* window, int count, const char** paths)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onDrop(count, paths);
            }
        }
        static void windowSizeCallback(GLFWwindow* window, int width, int height)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->mWidth  = width;
                app->mHeight = height;
            }
        }
        static void windowCloseCallback(GLFWwindow* window) { glfwSetWindowShouldClose(window, true); }

        void onReset()
        {
            for (auto& func : mOnResetFunc)
                func();
        }
        void onKey(int key, int scancode, int action, int mods)
        {
            for (auto& func : mOnKeyFunc)
                func(key, scancode, action, mods);
        }
        void onChar(unsigned int codepoint)
        {
            for (auto& func : mOnCharFunc)
                func(codepoint);
        }
        void onCharMods(int codepoint, unsigned int mods)
        {
            for (auto& func : mOnCharModsFunc)
                func(codepoint, mods);
        }
        void onMouseButton(int button, int action, int mods)
        {
            for (auto& func : mOnMouseButtonFunc)
                func(button, action, mods);
        }
        void onCursorPos(double xpos, double ypos)
        {
            for (auto& func : mOnCursorPosFunc)
                func(xpos, ypos);
        }
        void onCursorEnter(int entered)
        {
            for (auto& func : mOnCursorEnterFunc)
                func(entered);
        }
        void onScroll(double xoffset, double yoffset)
        {
            for (auto& func : mOnScrollFunc)
                func(xoffset, yoffset);
        }
        void onDrop(int count, const char** paths)
        {
            for (auto& func : mOnDropFunc)
                func(count, paths);
        }
        void onWindowSize(int width, int height)
        {
            for (auto& func : mOnWindowSizeFunc)
                func(width, height);
        }

    public: 
        GLFWwindow* mWindow {nullptr};
        int         mWidth {0};
        int         mHeight {0};

        bool pIsFocusMode {false};

        std::vector<onResetFunc>       mOnResetFunc;
        std::vector<onKeyFunc>         mOnKeyFunc;
        std::vector<onCharFunc>        mOnCharFunc;
        std::vector<onCharModsFunc>    mOnCharModsFunc;
        std::vector<onMouseButtonFunc> mOnMouseButtonFunc;
        std::vector<onCursorPosFunc>   mOnCursorPosFunc;
        std::vector<onCursorEnterFunc> mOnCursorEnterFunc;
        std::vector<onScrollFunc>      mOnScrollFunc;
        std::vector<onDropFunc>        mOnDropFunc;
        std::vector<onWindowSizeFunc>  mOnWindowSizeFunc;
        std::vector<onWindowCloseFunc> mOnWindowCloseFunc;
    };  
}