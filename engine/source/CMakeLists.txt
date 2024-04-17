# PackageProject.cmake will be used to make our target installable
set(TARGET_NAME DynastyRuntime)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(json_inlcude ${THIRD_PARTY_DIR}/json11)
set(json_lib_dir ${THIRD_PARTY_DIR}/lib/json11)

add_library(json11 ${json_inlcude}/json11.cpp)
set_target_properties(json11 PROPERTIES FOLDER "ThirdParty/json11" )

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")

#source_group(TREE "${ENGINE_ROOT_DIR}/schema" FILES ${SCHEMA_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library((${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES})
add_executable(${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Engine")

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")

# Link dependencies    
target_link_libraries(${TARGET_NAME} PUBLIC spdlog::spdlog)
target_link_libraries(${TARGET_NAME} PUBLIC stb)
target_link_libraries(${TARGET_NAME} PUBLIC glfw)
target_link_libraries(${TARGET_NAME} PUBLIC imgui)
target_link_libraries(${TARGET_NAME} PUBLIC glm)
target_link_libraries(${TARGET_NAME} PUBLIC assimp)
target_link_libraries(${TARGET_NAME} PUBLIC ${vulkan_lib})
target_link_libraries(${TARGET_NAME} PRIVATE $<BUILD_INTERFACE:json11>)

add_dependencies(${TARGET_NAME} ${SHADER_COMPILE_TARGET})

target_include_directories(
  ${TARGET_NAME}
  PUBLIC $<BUILD_INTERFACE:${vulkan_include}>)

# target_include_directories(
#   ${TARGET_NAME} 
#   PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/source/code>
#   $<INSTALL_INTERFACE:include/${TARGET_NAME}-${PROJECT_VERSION}>
# )

target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${json_inlcude}>
)

target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${THIRD_PARTY_DIR}/vulkanmemoryallocator/include>
)

add_compile_definitions("DYNASTY_ROOT_DIR=${BINARY_ROOT_DIR}")

target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E make_directory "${BINARY_ROOT_DIR}"
COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/" "${BINARY_ROOT_DIR}"
COMMAND ${CMAKE_COMMAND} -E remove_directory "${BINARY_ROOT_DIR}/${ENGINE_ASSET_DIR}"
)