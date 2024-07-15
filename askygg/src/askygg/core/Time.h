#pragma once
#include <stdint.h>

namespace askygg
{
	class Time
	{
	public:
		explicit Time(float time = 0.0f)
			: m_Time(time) {}

		explicit operator float() const { return m_Time; }

		static float DeltaTimeMilliseconds() { return s_DeltaTime * 1000.0f; }
		static float ElapsedMilliseconds() { return s_BackendElapsed * 1000.0f; }

		static void Tick();

		static float	DeltaTime() { return s_DeltaTime; }
		static float	Elapsed() { return s_BackendElapsed; }
		static uint64_t FrameCount() { return s_FrameCounter; }

	private:
		static float	s_BackendElapsed;
		static float	s_LastFrameTime;
		static float	s_DeltaTime;
		static uint64_t s_FrameCounter;
		float			m_Time = 0.0f;
	};
} // namespace askygg