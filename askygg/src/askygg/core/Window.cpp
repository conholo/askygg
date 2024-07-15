#include "askygg/core/Window.h"
#include "askygg/core/Assert.h"
#include "askygg/platform/windows/GLFWXPlatformWindow.h"

#ifdef E_PLATFORM_MACOS
	#include "askygg/platform/renderer_platform/metal/MetalWindow.h"
#endif

namespace askygg
{
	Ref<Window> Window::Create(const WindowProperties& props)
	{
#if defined(E_PLATFORM_LINUX) || defined(E_PLATFORM_WINDOWS)
		return CreateRef<GLFWXPlatformWindow>(props);
#elif defined(E_PLATFORM_MACOS)
		return CreateRef<MetalWindow>(props);
#else
		YGG_ASSERT(false, "Unknown platform!");
#endif
	}
} // namespace askygg