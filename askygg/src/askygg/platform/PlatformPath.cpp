#include "askygg/platform/PlatformPath.h"

#include "PlatformDetection.h"
#include "askygg/core/Assert.h"

#if defined(E_PLATFORM_WINDOWS)
	#include <windows.h>
#elif defined(E_PLATFORM_LINUX)
	#include <unistd.h>
#elif defined(E_PLATFORM_MACOS)
	#include <libgen.h>
	#include <mach-o/dyld.h>
#endif

#include <cstring>

std::string GetExecutablePath()
{
    char path[1024];
    std::memset(path, 0, sizeof(path));

#if defined(E_PLATFORM_WINDOWS)
    if (GetModuleFileName(NULL, path, sizeof(path)) == 0)
    {
        YGG_ASSERT(false, "Unable to find path to binary.");
    }
#elif defined(E_PLATFORM_LINUX)
    ssize_t res = readlink("/proc/self/exe", path, sizeof(path) - 1); // -1 to save space for null-terminator
    if (res == -1)
    {
        YGG_ASSERT(false, "Unable to find path to binary.");
    }
    else
    {
        path[res] = '\0';
    }
#elif defined(E_PLATFORM_MACOS)
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0)
    {
        YGG_ASSERT(false, "Unable to find path to binary.");
    }
#endif

    std::string strPath(path);
    std::size_t found = strPath.find_last_of("/\\");
    return strPath.substr(0, found);
}
