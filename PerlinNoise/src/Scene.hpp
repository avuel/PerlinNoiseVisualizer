#pragma once

#include <vector>

#include "PerlinNoise.hpp"
#include "AABB.hpp"

struct Scene
{
	std::vector<AABB> AABBs = {};
	AABB BoundingBox;

	PerlinNoiseGenerator PerlinNoiseGenerator{ 0, 256, 256, 16, 1, 1.0f };
	std::vector<double> Noise = {};
	size_t NoiseWidth = 0;
	size_t NoiseHeight = 0;
	int NoiseHeightScale = 1;

	bool GUI()
	{
		return PerlinNoiseGenerator.GUI(NoiseWidth, NoiseHeight);
	}
};