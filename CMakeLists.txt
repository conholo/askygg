cmake_minimum_required(VERSION 3.22)
project(askygg
        VERSION 1.0
        DESCRIPTION ""
        LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wno-deprecated-declarations)
endif()

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/askygg/third_party)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${THIRD_PARTY_DIR}/glad)
add_subdirectory(${THIRD_PARTY_DIR}/GLFW)
add_subdirectory(${THIRD_PARTY_DIR}/ImGui)
add_subdirectory(${THIRD_PARTY_DIR}/stbi)
add_subdirectory(${THIRD_PARTY_DIR}/yaml-cpp)
if(APPLE)
    add_subdirectory(${THIRD_PARTY_DIR}/metal)
endif()
add_subdirectory(askygg)
add_subdirectory(askygg_editor)