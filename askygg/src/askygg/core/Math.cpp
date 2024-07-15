
#include "askygg/core/Math.h"
#include <cstring>

namespace askygg
{
	float EaseIn(float t)
	{
		return t * t;
	}

	float Spike(float t)
	{
		return t <= 0.5f ? EaseIn(t / 0.5f) : t;
	}

	float Clamp(float value, float min, float max)
	{
		if (value > max)
			value = max;
		if (value < min)
			value = min;

		return value;
	}

	float Lerp(float start, float end, float t)
	{
		return start + t * (end - start);
	}

	float InverseLerp(float a, float b, float t)
	{
		return Clamp((t - a) / (b - a), 0, 1); // Clamp01 makes sure the result is between [0, 1]
	}

	float Remap(float value, float from1, float to1, float from2, float to2)
	{
		return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
	}

	glm::vec3 Remap(const glm::vec3& value, const glm::vec3& from1, const glm::vec3& to1,
		const glm::vec3& from2, const glm::vec3& to2)
	{
		float x = Remap(value.x, from1.x, to1.x, from2.x, to2.x);
		float y = Remap(value.y, from1.y, to1.y, from2.y, to2.y);
		float z = Remap(value.z, from1.z, to1.z, from2.z, to2.z);

		return { x, y, z };
	}

	double DoubleFromHalf(const uint16 half)
	{
		uint32 hi = uint32(half & 0x8000) << 16;
		uint16 abs = half & 0x7FFF;
		if (abs)
		{
			hi |= 0x3F000000 << uint16(abs >= 0x7C00);
			for (; abs < 0x400; abs <<= 1, hi -= 0x100000)
				;
			hi += uint32(abs) << 10;
		}
		const uint64 dbits = uint64(hi) << 32;
		double		 out;
		std::memcpy(&out, &dbits, sizeof(double));
		return out;
	}

} // namespace askygg