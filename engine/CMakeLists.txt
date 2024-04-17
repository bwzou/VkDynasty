
set(ENGINE_ROOT_DIR "${DYNASTY_ROOT_DIR}/engine")
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rdparty")
set(ENGINE_ASSET_DIR "/asset")

# 微软（MS）的VC运行库
if(MSVC) 
    add_compile_options("/MV")
endif()    

set(vulkan_include ${THIRD_PARTY_DIR}/VulkanSDK/include)

if(WIN32)
    set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/WIN32/vulkan-1.lib)
    set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/WIN32/glslangValidator.exe)
    add_compile_definitions("DYNASTY_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32")    
elseif(UNIX)    
    if(APPLE)
        set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/MacOS/libvulkan.1.dylib)
        set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/glslangValidator)
        add_compile_definitions("DYNASTY_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS")    
        add_compile_definitions("DYNASTY_VK_ICD_FILENAMES=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/MoltenVK_icd.json")    
    else()
        set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Linux/libvulkan.so.1)
        set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux/glslangValidator) 
        add_compile_definitions("DYNASTY_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux")      
    endif()
else()
    message(FATAL_ERROR "Unknown Platform")        
endif()

# 引入assimp
# include_directories(
#     "${THIRD_PARTY_DIR}/assimp/include"
#     "${THIRD_PARTY_DIR}/assimp/contrib"
# )

# assimp
# set(BUILD_SHARED_LIBS OFF)
# set(ASSIMP_NO_EXPORT ON)
# set(ASSIMP_BUILD_TESTS OFF)
# set(ASSIMP_INSTALL OFF)
# set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
# set(ASSIMP_BUILD_ZLIB ON)

# set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE)
# set(ASSIMP_BUILD_OBJ_IMPORTER TRUE)
# set(ASSIMP_BUILD_GLTF_IMPORTER TRUE)

# add_subdirectory(${THIRD_PARTY_DIR}/assimp)

set(SHADER_COMPILE_TARGET DynastyShaderCompile)

add_subdirectory(shader)
add_subdirectory(3rdparty)
add_subdirectory(source)

