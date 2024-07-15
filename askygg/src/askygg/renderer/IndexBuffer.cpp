#include "askygg/renderer/IndexBuffer.h"
#include "askygg/core/Assert.h"
#include "PlatformRenderAPI.h"
#include "askygg/platform/renderer_platform/opengl/OpenGLIndexBuffer.h"

namespace askygg
{
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t count)
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
				return CreateRef<OpenGLIndexBuffer>(count);
		}
		YGG_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
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
				return CreateRef<OpenGLIndexBuffer>(indices, count);
		}
		YGG_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

} // namespace askygg
