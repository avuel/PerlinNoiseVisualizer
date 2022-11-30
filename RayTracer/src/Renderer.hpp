#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "Camera.hpp"
#include "Ray.hpp"
#include "Scene.hpp"

#include "Walnut/Image.h"

class Renderer
{
public:
	struct Settings
	{
		bool Parallel = true;
	};

public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene &scene, const Camera &camera);

	std::shared_ptr<Walnut::Image> GetFinalImage();

	void ResetFrameIndex() { m_frameIndex = 1; }
	
	Settings &GetSettings() { return m_Settings; }
private:
	struct HitData
	{
		float HitDistance;
		glm::vec3 WorldNormal;
		glm::vec3 WorldPosition;

		uint32_t ObjectIndex;
		uint8_t ObjectType;

		float LightMultiplier;
		glm::vec3 Color;
		float SpecularExponent;
	};
	
	glm::vec4 PerPixel(uint32_t x, uint32_t y);

	// Function that casts a ray out into the world space
	HitData CastRay(const Ray &ray);

	// Function that returns the closest hit object for a casted ray
	HitData ClosestHit(const Ray &ray, float hitDistance, int objectIndex, uint8_t ObjectType, uint8_t objectFace);

	// Function that returns information that a casted ray missed (did not intersect) any object
	HitData Miss(const Ray &ray);

private:
	const Scene *m_ActiveScene = nullptr;
	const Camera *m_ActiveCamera = nullptr;
	Settings m_Settings;

	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t *m_ImageData = nullptr;

	uint32_t m_frameIndex = 1;
};

