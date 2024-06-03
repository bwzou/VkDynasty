
#include "DynastyEditorUI.h"


void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    std::cout << "Do nothing!" << std::endl;
}

void mouseCallback(GLFWwindow *window, double xPos, double yPos) {
    std::cout << "Do nothing!" << std::endl;
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
    std::cout << "Do nothing!" << std::endl;
}

void processInput(GLFWwindow *window) {
     std::cout << "Do nothing!" << std::endl;
}


int DynastyEditorUI::initWindow() {
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
    window_ = glfwCreateWindow(WIDTH, HEIGHT, "MiniRenderer", nullptr, nullptr);
    if (!window_) {
        throw std::runtime_error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window_);
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSetCursorPosCallback(window_, mouseCallback);
    glfwSetScrollCallback(window_, scrollCallback);

    return 1;
}


void DynastyEditorUI::cleanup() {
    glfwDestroyWindow(window_);

    glfwTerminate();
}

