file(GLOB md5_sources CONFIGURE_DEPENDS  "${CMAKE_CURRENT_SOURCE_DIR}/md5/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/md5/*.cpp")
add_library(md5 INTERFACE ${md5_sources})
target_include_directories(md5 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/md5)