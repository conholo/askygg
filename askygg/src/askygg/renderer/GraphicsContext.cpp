#include "askygg/renderer/GraphicsContext.h"
#include "askygg/core/Assert.h"

#include "askygg/renderer/PlatformRenderAPI.h"
#include "askygg/platform/renderer_platform/opengl/OpenGLGraphicsContext.h"

namespace askygg
{
	Ref<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (PlatformRenderAPI::GetPlatformRendererType())
		{
			case PlatformRenderAPI::API::None:
				YGG_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			case PlatformRenderAPI::API::Vulkan:
				YGG_ASSERT(false, "RendererAPI::Vulkan is currently not supported!");
				return nullptr;
			case PlatformRenderAPI::API::Metal:
				YGG_ASSERT(false, "RendererAPI::Metal is currently not supported!");
				return nullptr;
			case PlatformRenderAPI::API::OpenGL:
				return CreateRef<OpenGLGraphicsContext>(static_cast<GLFWwindow*>(window));
		}

		YGG_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
} // namespace askygg