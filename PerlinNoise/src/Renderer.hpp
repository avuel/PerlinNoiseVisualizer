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
		bool Parallel = true;
		bool Noise = false;
		bool BoundingBox = false;
		bool Clip = true;
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
		float HitDistance;
		glm::vec3 WorldNormal;
		glm::vec3 WorldPosition;
		uint32_t HitPixel;
	};
	
	glm::vec4 PerPixel(const uint32_t &pixel);

	// Function that casts a ray out into the world space
	HitData CastRay(const Ray &ray);

	// Function that returns the closest hit object for a casted ray
	HitData ClosestHit(const Ray &ray, const float &hitDistance, const glm::vec3 &hitNormal, const size_t &hitPixel);

	// Function that returns information that a casted ray missed (did not intersect) any object
	HitData Miss(const Ray &ray);

private:
	Scene *m_ActiveScene = nullptr;
	const Camera *m_ActiveCamera = nullptr;
	Settings m_Settings;

	std::shared_ptr<Walnut::Image> m_FinalImage;
	glm::vec3 *m_NormalBuffer = nullptr;
	uint32_t *m_ColorBuffer = nullptr;
	size_t m_Width = 0;
	size_t m_Height = 0;
};

