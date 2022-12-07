#pragma once

#include <vector>

#include "PerlinNoise.hpp"
#include "AABB.hpp"
#include "OcTree.hpp"

struct Scene
{
	OcTree *ocTree = new OcTree();
	PerlinNoiseGenerator PerlinNoiseGenerator{ 0, 32, 32, 16, 2, 0.15f };
	std::vector<double> Noise = {};

	size_t NoiseWidth = 0;
	size_t NoiseHeight = 0;

	bool GUI()
	{
		return PerlinNoiseGenerator.GUI(NoiseWidth, NoiseHeight);
	}

	NoiseSettings *GetNoiseSettings() { return PerlinNoiseGenerator.GetNoiseSettings(); }
	void SetNoiseHeight(const int   &height)  { PerlinNoiseGenerator.SetHeight(height); }
	void SetNoiseWater (const float &water )  { PerlinNoiseGenerator.SetWater(water);   }
	void SetNoiseSand  (const float &sand  )  { PerlinNoiseGenerator.SetSand(sand);     }
	void SetNoiseStone (const float &stone )  { PerlinNoiseGenerator.SetStone(stone);   }
	void SetNoiseSnow  (const float &snow  )  { PerlinNoiseGenerator.SetSnow(snow);     }
	const int GetNoiseHeight() const { return PerlinNoiseGenerator.GetNoiseHeight(); }
	

	~Scene()
	{
		// No memory leak please
		delete ocTree;
	}
};