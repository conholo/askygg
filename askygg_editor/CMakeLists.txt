cmake_minimum_required(VERSION 3.22)

set(NAME askygg_editor)
set(SOURCE_DIR src/)

add_executable(${NAME}
        src/EditorApplication.cpp
        src/Layers/EditorLayer.cpp
        src/Layers/HeadlessLayer.cpp

        src/ImageEditor/ImageEditor.cpp
        src/ImageEditor/ImagePass.cpp

        src/ImageEditor/Passes/SobelPass.cpp
        src/ImageEditor/Passes/MultiPassBloomPass.cpp
        src/ImageEditor/Passes/ContrastBrightnessPass.cpp
        src/ImageEditor/Passes/RadialBlurPass.cpp
        src/ImageEditor/Passes/ChromaticAberrationPass.cpp
        src/ImageEditor/Passes/BarrelDistortionPass.cpp
        src/ImageEditor/Passes/VignettePass.cpp
        src/ImageEditor/Passes/SharpenPass.cpp
        src/ImageEditor/Passes/HSVAdjustmentPass.cpp
        src/ImageEditor/Passes/OutputComputePass.cpp
        src/ImageEditor/Passes/LinearizePass.cpp
        src/ImageEditor/Passes/RadialBloomPass.cpp
)

target_include_directories(${NAME} PRIVATE ${SOURCE_DIR})
target_include_directories(${NAME} PRIVATE ${SOURCE_DIR}/ImageEditor/)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/askygg/src/)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/askygg/src/askygg)
target_include_directories(${NAME} PRIVATE ${THIRD_PARTY_DIR}/spdlog/include)
target_include_directories(${NAME} PRIVATE ${THIRD_PARTY_DIR}/entt/include)
target_include_directories(${NAME} PRIVATE ${THIRD_PARTY_DIR}/ImGui)

if(APPLE)
    set(PLATFORM_INCLUDE_DIRS
            third_party/metal/metal-cpp
            third_party/metal/metal-cpp-extensions)
    target_include_directories(${NAME} PRIVATE ${PLATFORM_INCLUDE_DIRS})
endif()

target_link_libraries(${NAME} askygg)

set_target_properties(${NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)

if (UNIX)
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/${NAME}/assets)
        add_custom_command(TARGET ${NAME} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/${NAME}/assets)
    endif()
else()
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/${NAME}/assets)
        file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/${NAME}/assets" DestinationDirectory)
        file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets" SourceDirectory)
        add_custom_command(TARGET ${NAME} PRE_BUILD
                COMMAND cmd.exe /c mklink /J
                "${DestinationDirectory}" "${SourceDirectory}")
    endif()
endif()

