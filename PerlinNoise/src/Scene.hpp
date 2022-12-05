#pragma once

#include <vector>

#include "PerlinNoise.hpp"
#include "AABB.hpp"
#include "OcTree.hpp"

struct Scene
{
	OcTree OcTree;
	PerlinNoiseGenerator PerlinNoiseGenerator{ 0, 32, 32, 16, 2, 0.15f };
	std::vector<double> Noise = {};

	size_t NoiseWidth = 0;
	size_t NoiseHeight = 0;
	int NoiseHeightScale = 32;

	bool GUI()
	{
		return PerlinNoiseGenerator.GUI(NoiseWidth, NoiseHeight);
	}
};