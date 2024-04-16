#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <fstream>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <array>
#include <limits>
#include <optional>
#include <set>
#include <unordered_map>
#include "DynastyEngine.h"

#define DYNASTY_XSTR(s) DYNASTY_STR(s)
#define DYNASTY_STR(s) #s 

int main(int argc, char** argv) {
    DynastyEngine app;

#if defined(__GNUC__)
    // https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
    #if defined(__linux__)
        char const* vk_layer_path = DYNASTY_XSTR(DYNASTY_VK_LAYER_PATH);
        setenv("VK_LAYER_PATH", vk_layer_path, 1);
    #elif defined(__MACH__)
        // https://developer.apple.com/library/archive/documentation/Porting/Conceptual/PortingUnix/compiling/compiling.html
        char const* vk_layer_path    = DYNASTY_XSTR(DYNASTY_VK_LAYER_PATH);
        char const* vk_icd_filenames = DYNASTY_XSTR(DYNASTY_VK_ICD_FILENAMES);
        setenv("VK_LAYER_PATH", vk_layer_path, 1);
        setenv("VK_ICD_FILENAMES", vk_icd_filenames, 1);
    #else
        #error Unknown Platform
    #endif
#elif defined(_MSC_VER)
    // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
    char const* vk_layer_path = DYNASTY_XSTR(TUTORIAL_VK_LAYER_PATH);
    SetEnvironmentVariableA("VK_LAYER_PATH", vk_layer_path);
#else
    #error Unknown Compiler
#endif

    // std::filesystem::path executable_path(argv[0]);
    // std::filesystem::path config_file_path = executable_path.parent_path() / "PiccoloEditor.ini";

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

