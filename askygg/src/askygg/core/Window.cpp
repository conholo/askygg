#include "askygg/core/Window.h"
#include "askygg/core/Assert.h"
#include "askygg/platform/windows/GLFWWindow.h"

#ifdef E_PLATFORM_MACOS
	#include "askygg/platform/renderer_platform/metal/MetalWindow.h"
#endif

namespace askygg
{
	Ref<Window> Window::Create(const WindowProperties& props)
	{
		// Temp macOS here - adding Vulkan Support to MacOS - deal with native windowing later
#if defined(E_PLATFORM_LINUX) || defined(E_PLATFORM_WINDOWS) || defined(E_PLATFORM_MACOS)
		return CreateRef<GLFWWindow>(props);
#elif defined(E_PLATFORM_MACOS)
		return CreateRef<MetalWindow>(props);
#else
		YGG_ASSERT(false, "Unknown platform!");
#endif
	}
} // namespace askygg