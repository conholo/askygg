#include "askygg/renderer/VertexBuffer.h"
#include "askygg/core/Memory.h"
#include "PlatformRenderAPI.h"
#include "askygg/core/Assert.h"
#include "askygg/platform/renderer_platform/opengl/OpenGLVertexBuffer.h"

namespace askygg
{
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
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
				return CreateRef<OpenGLVertexBuffer>(size);
		}
		YGG_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
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
				return CreateRef<OpenGLVertexBuffer>(vertices, size);
		}
		YGG_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
} // namespace askygg
