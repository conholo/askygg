cmake_minimum_required(VERSION 3.22)
set(NAME askygg)

if(APPLE)
    set(PLATFORM_SRC
            src/${NAME}/platform/mac_os/MacOSApplication.cpp
            src/${NAME}/platform/renderer_platform/metal/MetalWindow.cpp
            src/${NAME}/platform/renderer_platform/metal/MetalDevice.cpp
            src/${NAME}/platform/renderer_platform/metal/MetalRenderer.cpp
            src/${NAME}/platform/renderer_platform/metal/MetalViewDelegate.cpp
            src/${NAME}/platform/renderer_platform/metal/MetalApplicationDelegate.cpp
            src/${NAME}/platform/renderer_platform/metal/MetalRenderPass.cpp
            src/${NAME}/platform/renderer_platform/metal/MetalFramebuffer.cpp
            src/${NAME}/platform/windows/GLFWWindow.cpp)
elseif(WIN32)
    set(PLATFORM_SRC src/${NAME}/platform/linux/LinuxApplication.cpp src/${NAME}/platform/windows/GLFWWindow.cpp)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_SRC src/${NAME}/platform/linux/LinuxApplication.cpp src/${NAME}/platform/windows/GLFWWindow.cpp)
endif()

# 1. Set VULKAN_SDK_PATH in .env.cmake to target specific vulkan version
if (DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
    set(Vulkan_FOUND "True")
else ()
    find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
    message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif ()
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else ()
    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif ()

# assume built-in pthreads on MacOS
IF (APPLE)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
ENDIF ()


add_library(${NAME} STATIC
    src/${NAME}/core/Application.cpp
    src/${NAME}/core/Input.cpp
    src/${NAME}/core/LayerStack.cpp
    src/${NAME}/core/Log.cpp
    src/${NAME}/core/Math.cpp
    src/${NAME}/core/Random.cpp
    src/${NAME}/core/Time.cpp
    src/${NAME}/core/UUID.cpp
    src/${NAME}/core/Window.cpp
    src/${NAME}/core/EngineUtils.cpp

    src/${NAME}/imgui/ImGuiLayer.cpp

    src/${NAME}/renderer/Camera.cpp
    src/${NAME}/renderer/Framebuffer.cpp
    src/${NAME}/renderer/IndexBuffer.cpp
    src/${NAME}/renderer/Material.cpp
    src/${NAME}/renderer/RenderCommand.cpp
    src/${NAME}/renderer/Renderer.cpp
    src/${NAME}/renderer/RenderPass.cpp
    src/${NAME}/renderer/Shader.cpp
    src/${NAME}/renderer/Texture.cpp
    src/${NAME}/renderer/TextureUtils.cpp
    src/${NAME}/renderer/UniformBuffer.cpp
    src/${NAME}/renderer/VertexArray.cpp
    src/${NAME}/renderer/VertexBuffer.cpp
    src/${NAME}/renderer/Mesh.cpp
    src/${NAME}/renderer/RenderPass.cpp
    src/${NAME}/renderer/GraphicsContext.cpp
    src/${NAME}/renderer/PlatformRenderer.cpp

    src/${NAME}/scene/Entity.cpp
    src/${NAME}/scene/Scene.cpp
    
    src/${NAME}/ui/Dockspace.cpp
    src/${NAME}/ui/PropertyDrawer.cpp
    src/${NAME}/ui/StatisticsPanel.cpp
    src/${NAME}/ui/UIDrawerHelpers.cpp
    src/${NAME}/ui/Viewport.cpp

    src/${NAME}/platform/renderer_platform/opengl/OpenGLRenderer.cpp
    src/${NAME}/platform/renderer_platform/opengl/OpenGLGraphicsContext.cpp
    src/${NAME}/platform/renderer_platform/opengl/OpenGLVertexBuffer.cpp
    src/${NAME}/platform/renderer_platform/opengl/OpenGLIndexBuffer.cpp
    src/${NAME}/platform/renderer_platform/opengl/OpenGLVertexArray.cpp
    src/${NAME}/platform/renderer_platform/opengl/OpenGLFramebuffer.cpp
    src/${NAME}/platform/renderer_platform/opengl/OpenGLShader.cpp

    src/${NAME}/platform/renderer_platform/vulkan/VulkanGraphicsContext.cpp
    src/${NAME}/platform/renderer_platform/vulkan/VulkanDevice.cpp
    src/${NAME}/platform/renderer_platform/vulkan/VulkanSwapchain.cpp
    src/${NAME}/platform/renderer_platform/vulkan/VulkanDescriptors.cpp
    src/${NAME}/platform/renderer_platform/vulkan/VulkanFramebuffer.cpp
    src/${NAME}/platform/renderer_platform/vulkan/VulkanImage.cpp
    src/${NAME}/platform/renderer_platform/vulkan/VulkanTexture.cpp
    src/${NAME}/platform/renderer_platform/vulkan/VulkanGraphicsPipeline.cpp
    src/${NAME}/platform/renderer_platform/vulkan/VulkanRenderer.cpp
    src/${NAME}/platform/renderer_platform/vulkan/VulkanComputePipeline.cpp


    src/${NAME}/platform/PlatformPath.cpp
    ${PLATFORM_SRC})

set_target_properties(${NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

if(APPLE)
    set(PLATFORM_INCLUDE_DIRS
            third_party/metal/metal-cpp
            third_party/metal/metal-cpp-extensions)
endif()


find_package(OpenGL REQUIRED)
target_include_directories(${NAME} PUBLIC src/)
target_include_directories(${NAME} PUBLIC third_party/glad/include/)
target_include_directories(${NAME} PUBLIC third_party/GLFW/include/)
target_include_directories(${NAME} PUBLIC third_party/entt/include/)
target_include_directories(${NAME} PUBLIC third_party/stbi/include/)
target_include_directories(${NAME} PUBLIC third_party/yaml-cpp/include/)
target_include_directories(${NAME} PUBLIC third_party/spdlog/include/)
target_include_directories(${NAME} PUBLIC third_party/glm/)
target_include_directories(${NAME} PUBLIC third_party/ImGui/)
target_include_directories(${NAME} PUBLIC third_party/entt/include)
target_include_directories(${NAME} PUBLIC ${OPENGL_INCLUDE_DIRS})
target_include_directories(${NAME} PUBLIC ${PLATFORM_INCLUDE_DIRS})
target_include_directories(${NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})

if(APPLE)
    set(PLATFORM_LINK_LIBS METAL_CPP)
endif()


target_link_libraries(${NAME} glfw glad ImGui stbi yaml-cpp ${OPENGL_LIBRARIES} ${PLATFORM_LINK_LIBS} ${Vulkan_LIBRARIES})

############## Build SHADERS #######################

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS
        ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
        /usr/bin
        /usr/local/bin
        ${VULKAN_SDK_PATH}/Bin
        ${VULKAN_SDK_PATH}/Bin32
        $ENV{VULKAN_SDK}/Bin/
        $ENV{VULKAN_SDK}/Bin32/
)

# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/assets/shaders/*.frag"
        "${PROJECT_SOURCE_DIR}/assets/shaders/*.vert"
        "${PROJECT_SOURCE_DIR}/assets/shaders/*.comp"
)

foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/assets/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV} -g
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)