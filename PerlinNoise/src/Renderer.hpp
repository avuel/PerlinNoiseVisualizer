#pragma once

#include <memory>

#include "Camera.hpp"
#include "Ray.hpp"
#include "Scene.hpp"
#include "PerlinNoise.hpp"

#include "Walnut/Image.h"


class Renderer
{
public:
	struct Settings
	{
		bool  Parallel = true;
		bool  Noise    = false;
		bool  OcTree   = true;
	};

public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(Scene &scene, const Camera &camera);

	std::shared_ptr<Walnut::Image> GetFinalImage();
	
	Settings &GetSettings() { return m_Settings; }
private:
	struct HitData
	{
		float HitTime;
		glm::vec3 WorldPosition;
	};
	
	glm::vec4 PerPixel(const uint32_t &pixel);

	// Function that casts a ray out into the world space
	HitData CastRay(const Ray &ray);

	// Function that returns the closest hit object for a casted ray
	HitData ClosestHit(const Ray &ray, const float &hitTime, const glm::vec3 &hitPoint);

	// Function that returns information that a casted ray missed (did not intersect) any object
	HitData Miss(const Ray &ray);

private:
	Scene *m_ActiveScene = nullptr;
	const Camera *m_ActiveCamera = nullptr;
	Settings m_Settings;

	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t *m_ColorBuffer = nullptr;

	size_t m_Width = 0;
	size_t m_Height = 0;

	NoiseSettings m_NoiseSettings;
	//size_t m_NoiseHeight = 32;
};

