#include <random>
#include "askygg/core/Random.h"

namespace askygg
{
	std::mt19937 Random::s_Gen;

	void Random::Seed(int seed)
	{
		s_Gen.seed(seed);
	}

	void Random::Initialize()
	{
		std::random_device rd;
		s_Gen.seed(rd());
	}

	float Random::RandomRange(float min, float max)
	{
		std::uniform_real_distribution<float> dist(min, max);
		return dist(s_Gen);
	}

	int Random::RandomRange(int min, int max)
	{
		std::uniform_int_distribution<int> dist(min, max);
		return dist(s_Gen);
	}

	float Random::RandomNormal(float mean, float stddev)
	{
		std::normal_distribution<float> dist(mean, stddev);
		return dist(s_Gen);
	}

	bool Random::RandomBool(double probability)
	{
		std::bernoulli_distribution dist(probability);
		return dist(s_Gen);
	}
} // namespace askygg