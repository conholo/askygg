#pragma once
#include <spdlog/spdlog.h>
#include "askygg/core/Memory.h"
#include "glm/gtx/string_cast.hpp"

namespace askygg
{
	class Log
	{
	public:
		static void					Init();
		static Ref<spdlog::logger>& GetLogger() { return s_Logger; }
		template <typename... Args>
		static void PrintAssertMessage(std::string_view Prefix, Args&&... args);

	private:
		static Ref<spdlog::logger> s_Logger;
	};
} // namespace askygg

template <typename OStream>
OStream& operator<<(OStream& os, const glm::vec3& vec)
{
	return os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ')';
}

template <typename OStream>
OStream& operator<<(OStream& os, const glm::vec4& vec)
{
	return os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ')';
}

namespace askygg
{
	template <typename... Args>
	void Log::PrintAssertMessage(std::string_view Prefix, Args&&... args)
	{
		GetLogger()->error("{0}: {1}", Prefix, fmt::format(std::forward<Args>(args)...));
	}
} // namespace askygg

#define YGG_LOG_TRACE(...) ::askygg::Log::GetLogger()->trace(__VA_ARGS__)
#define YGG_LOG_INFO(...) ::askygg::Log::GetLogger()->info(__VA_ARGS__)
#define YGG_LOG_WARN(...) ::askygg::Log::GetLogger()->warn(__VA_ARGS__)
#define YGG_LOG_ERROR(...) ::askygg::Log::GetLogger()->error(__VA_ARGS__)
#define YGG_LOG_CRITICAL(...) ::askygg::Log::GetLogger()->critical(__VA_ARGS__)
