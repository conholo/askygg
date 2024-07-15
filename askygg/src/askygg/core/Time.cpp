
#include "askygg/core/Time.h"
#include <GLFW/glfw3.h>

namespace askygg
{
	float	 Time::s_LastFrameTime = 0.0f;
	float	 Time::s_DeltaTime = 0.0f;
	float	 Time::s_BackendElapsed = 0.0f;
	uint64_t Time::s_FrameCounter = 0;

	void Time::Tick()
	{
		s_BackendElapsed = static_cast<float>(glfwGetTime());
		s_DeltaTime = s_BackendElapsed - s_LastFrameTime;
		s_LastFrameTime = s_BackendElapsed;
		s_FrameCounter++;
	}
} // namespace askygg