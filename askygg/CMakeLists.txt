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
            src/${NAME}/platform/renderer_platform/metal/MetalFramebuffer.cpp)
elseif(WIN32)
    set(PLATFORM_SRC src/${NAME}/platform/linux/LinuxApplication.cpp src/${NAME}/platform/windows/GLFWXPlatformWindow.cpp)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_SRC src/${NAME}/platform/linux/LinuxApplication.cpp src/${NAME}/platform/windows/GLFWXPlatformWindow.cpp)
endif()

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

if(APPLE)
    set(PLATFORM_LINK_LIBS METAL_CPP)
endif()


target_link_libraries(${NAME} glfw glad ImGui stbi yaml-cpp ${OPENGL_LIBRARIES} ${PLATFORM_LINK_LIBS})
