#pragma once

#include <glm/glm.hpp>

typedef unsigned short	   uint16;
typedef unsigned int	   uint32;
typedef unsigned long long uint64;

namespace askygg
{
	float	  EaseIn(float t);
	float	  Spike(float t);
	float	  Clamp(float value, float min, float max);
	float	  Lerp(float start, float end, float t);
	float	  InverseLerp(float a, float b, float t);
	float	  Remap(float value, float from1, float to1, float from2, float to2);
	glm::vec3 Remap(const glm::vec3& value, const glm::vec3& from1, const glm::vec3& to1,
		const glm::vec3& from2, const glm::vec3& to2);
	double	  DoubleFromHalf(uint16 half);
} // namespace askygg