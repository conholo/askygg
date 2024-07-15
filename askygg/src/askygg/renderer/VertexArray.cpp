#include "askygg/renderer/VertexArray.h"
#include "PlatformRenderAPI.h"
#include "askygg/core/Assert.h"
#include "askygg/platform/renderer_platform/opengl/OpenGLVertexArray.h"

namespace askygg
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (PlatformRenderAPI::GetPlatformRendererType())
		{
			case PlatformRenderAPI::API::None:
				YGG_ASSERT(false, "RendererAPI::None is currently not supported!");
			case PlatformRenderAPI::API::Vulkan:
				YGG_ASSERT(false, "RendererAPI::Vulkan is currently not supported!");
			case PlatformRenderAPI::API::Metal:
				YGG_ASSERT(false, "RendererAPI::Metal is currently not supported!");
			case PlatformRenderAPI::API::OpenGL:
				return CreateRef<OpenGLVertexArray>();
		}
		YGG_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
} // namespace askygg
