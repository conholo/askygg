#include "askygg/core/Assert.h"
#include "askygg/renderer/Framebuffer.h"
#include "PlatformRenderAPI.h"
#include "askygg/platform/renderer_platform/opengl/OpenGLFramebuffer.h"

#ifdef E_PLATFORM_MACOS
	#include "askygg/platform/renderer_platform/metal/MetalFramebuffer.h"
#endif

namespace askygg
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (PlatformRenderAPI::GetPlatformRendererType())
		{
			case PlatformRenderAPI::API::None:
				YGG_ASSERT(false, "RendererAPI::None is currently not supported!");
			case PlatformRenderAPI::API::Vulkan:
				YGG_ASSERT(false, "RendererAPI::Vulkan is currently not supported!");
			case PlatformRenderAPI::API::Metal:
#ifdef E_PLATFORM_MACOS
				return CreateRef<MetalFramebuffer>(spec);
#endif
			case PlatformRenderAPI::API::OpenGL:
				return CreateRef<OpenGLFramebuffer>(spec);
		}
		YGG_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
} // namespace askygg
