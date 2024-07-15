#pragma once

#include "askygg/core/Utility.h"
#include "askygg/core/Log.h"

#define ENABLE_ASSERTS

#if defined(E_PLATFORM_WINDOWS)
	#define E_DEBUGBREAK() __debugbreak()
#elif defined(E_PLATFORM_LINUX)
	#include <csignal>
	#define E_DEBUGBREAK() raise(SIGTRAP)
#elif defined(E_PLATFORM_MACOS)
	#define E_DEBUGBREAK() __builtin_trap()
#else
	#error "Platform doesn't support debugbreak yet!"
#endif

#ifdef ENABLE_ASSERTS
	#define YGG_ASSERT_MESSAGE_INTERNAL(...) \
		::askygg::Log::PrintAssertMessage("Assertion Failed", __VA_ARGS__)
	#define YGG_ASSERT(condition, ...)                    \
		{                                                 \
			if (!(condition))                             \
			{                                             \
				YGG_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); \
				E_DEBUGBREAK();                           \
                std::exit(1);                             \
            }                                             \
		}
#else
	#define YGG_ASSERT(condition, ...)
#endif
