#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <iostream>
#include "../function/Config.h"
// #include "constant.h"


const uint32_t WIDTH = 1000;
const uint32_t HEIGHT = 800;


static void glfwErrorCallback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


class DynastyEditorUI{
private:
    /* data */
    GLFWwindow* window_;
    Config &config_;

public:
    DynastyEditorUI(Config &config) : config_(config){

    }
    ~DynastyEditorUI(){
        
    }

    GLFWwindow* window() {
        return window_;
    }

    int initWindow();
    void cleanup();
};
