cmake_minimum_required(VERSION 3.22)
add_library(ImGui STATIC 
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    ImGuizmo.cpp

    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp)

target_include_directories(ImGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${OPENGL_INCLUDE_DIRS} ${THIRD_PARTY_DIR}/GLFW/include/)