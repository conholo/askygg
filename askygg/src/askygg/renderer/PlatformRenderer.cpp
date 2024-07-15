#include "askygg/renderer/PlatformRenderAPI.h"
#include "askygg/core/Assert.h"

#include "askygg/platform/renderer_platform/opengl/OpenGLRenderer.h"

#ifdef E_PLATFORM_MAC_OS
	#include "askygg/platform/renderer_platform/metal/MetalRenderer.h"
#endif

namespace askygg
{
	PlatformRenderAPI::API PlatformRenderAPI::s_API;
	Ref<PlatformRenderAPI> PlatformRenderAPI::s_RenderAPI = nullptr;

	void PlatformRenderAPI::InitializePlatformRendererType()
	{
#if defined(E_PLATFORM_LINUX) || defined(E_PLATFORM_WINDOWS)
		s_API = PlatformRenderAPI::API::OpenGL;
#elif defined(E_PLATFORM_MACOS)
		s_API = PlatformRenderAPI::API::Metal;
#else
		s_API = PlatformRenderAPI::API::None;
#endif

		switch (s_API)
		{
			case PlatformRenderAPI::API::None:
				YGG_ASSERT(false, "RendererAPI::None is not supported!");
				break;
			case PlatformRenderAPI::API::OpenGL:
				s_RenderAPI = CreateRef<OpenGLRenderer>();
				break;
			case PlatformRenderAPI::API::Metal:
#ifdef E_PLATFORM_MACOS
				s_RenderAPI = CreateRef<MetalRenderer>();
				break;
#endif
			case PlatformRenderAPI::API::Vulkan:
				YGG_ASSERT(false, "Vulkan is not currently supported!");
				break;
		}
	}
} // namespace askygg