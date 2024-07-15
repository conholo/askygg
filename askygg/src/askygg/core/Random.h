#pragma once
#include <random>
#include <ctime>

namespace askygg
{
	class Random
	{
	public:
		static void	 Seed(int seed);
		static void	 Initialize();
		static float RandomRange(float min, float max);
		static int	 RandomRange(int min, int max);
		static float RandomNormal(float mean = 0.0, float stddev = 1.0);
		static bool	 RandomBool(double probability = 0.5);

	private:
		static std::mt19937 s_Gen;
	};
} // namespace askygg