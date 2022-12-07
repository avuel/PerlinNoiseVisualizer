#include "Renderer.hpp"
#include "Walnut/Random.h"

#include <numeric>
#include <algorithm>
#include <execution>
#include <iostream>

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4 &color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		// 0xAABBGGRR
		return (uint32_t)((a << 24) | (b << 16) | (g << 8) | r);
	}

	// Fast Ray-AABB intersection algorithm (using the slab method)
	// Code adapted from https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d
	static float RayAABBIntersection(const Ray &ray, const glm::vec3 &boxMin, const glm::vec3 &boxMax)
	{
		glm::vec3 tMin = (boxMin - ray.Origin) / ray.Direction;
		glm::vec3 tMax = (boxMax - ray.Origin) / ray.Direction;
		glm::vec3 t1 = min(tMin, tMax);
		glm::vec3 t2 = max(tMin, tMax);
		float tNear = std::max(std::max(t1.x, t1.y), t1.z);
		float tFar = std::min(std::min(t2.x, t2.y), t2.z);

		// If tNear > tFar, we did not have an intersection so return -1.0f
		if (tNear > tFar)
			return -1.0f;

		// If we had an intersection but tNear is behind us, we are inside the box so just set t to max float value
		if (tNear < 0.0f)
			return std::numeric_limits<float>::max();

		return tNear;
	};

	// Scans the chunk to see if we intersect it, if we do we perform this check for its children
	static void ScanChunks(const Ray &ray, OcTree *chunk, float &hitTime, glm::vec3 &hitPoint)
	{
		// If the chunk does not contain a point we do not want to intersect it
		if (chunk->GetPointCount() == 0)
			return;

		// Intersect against the current oct
		const AABB &oct = chunk->GetOct();
		std::vector<glm::vec3> boundaries = oct.GetBoundaries();
		float t = RayAABBIntersection(ray, boundaries[0], boundaries[1]);

		// Return if we did not have an intsection
		if (t < 0.0f)
			return;

		// Update the time if the box is size 1x1x1
		if (chunk->GetSize() == 1 && t < hitTime)
		{
			hitTime = t;
			hitPoint = chunk->GetPoints()[0];
		}
			

		// If we hit the chunk, check the children of the chunk
		for (const auto &child : chunk->GetChildren())
			ScanChunks(ray, child, hitTime, hitPoint);
	}
}



void Renderer::OnResize(uint32_t width, uint32_t height)
{
	m_Width = width;
	m_Height = height;

	if (m_FinalImage)
	{
		// Do not resize the image
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA); 
	}

	delete[] m_ColorBuffer;
	m_ColorBuffer = new uint32_t[width * height];
}

void Renderer::Render(Scene &scene, const Camera &camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;
	
	if (m_ActiveScene->GUI())
	{
		// Update the noise and get its maximum dimension
		m_NoiseSettings = *scene.GetNoiseSettings();
		m_ActiveScene->Noise = m_ActiveScene->PerlinNoiseGenerator.GetNoise();
		uint32_t size = std::max((int)std::max(m_ActiveScene->NoiseWidth, m_ActiveScene->NoiseHeight), m_NoiseSettings.Height);

		// Set the points up for the scene
		std::vector<glm::vec3> points(m_ActiveScene->Noise.size());

		for (int i = 0; i < points.size(); i++)
		{
			float x = i % m_ActiveScene->NoiseWidth;
			float y = int(m_ActiveScene->Noise[i] * m_NoiseSettings.Height);
			float z = i / m_ActiveScene->NoiseWidth;
			
			points[i] = glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
		}

		// Generate the OcTree for the scene
		m_ActiveScene->ocTree->Generate(size, points);

		std::cout << "Noise and OcTree Generated" << '\n';
		std::cout << "Dimension: " << size << 'x' << size << 'x' << size << '\n';
		std::cout << "Noise Data Count: " << m_ActiveScene->Noise.size() << '\n';
		std::cout << "Scene Octs Count: " << m_ActiveScene->ocTree->GetOctCount() << '\n';
		std::cout << '\n';
	}

	const std::vector<glm::vec3> rayDirections = camera.GetRayDirections();
	// https://stackoverflow.com/questions/17694579/use-stdfill-to-populate-vector-with-increasing-numbers
	// Code that creates and fills a vector of size n where the elements are 0,1,2,...,n - 1
	std::vector<int> pixels(m_Width * m_Height); 
	std::iota (std::begin(pixels), std::end(pixels), 0);


	// This if, else block will send out the rays for each pixel and get the color of that pixel
	if (m_Settings.Parallel) 
	{
		std::for_each(std::execution::par, pixels.begin(), pixels.end(), [this](const int &pixel)
			{
				uint32_t x = pixel % m_Width;
				uint32_t y = pixel / m_Width;
				glm::vec4 color = PerPixel(x + y * m_Width);

				color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
				m_ColorBuffer[x + y * m_Width] = Utils::ConvertToRGBA(color);
			});
	}

	// Non parallelized for loops
	else
	{
		for (uint32_t y = 0; y < m_Height; y++)
		{
			for (uint32_t x = 0; x < m_Width; x++)
			{
				glm::vec4 color = PerPixel(x + y * m_Width);

				color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
				m_ColorBuffer[x + y * m_Width] = Utils::ConvertToRGBA(color);
			}
		}
	}

	// Set the image data
	m_FinalImage->SetData(m_ColorBuffer);
}

std::shared_ptr<Walnut::Image> Renderer::GetFinalImage()
{
	return m_FinalImage;
}

glm::vec4 Renderer::PerPixel(const uint32_t &pixel)
{
	glm::vec3 cameraPosition = m_ActiveCamera->GetPosition();
	glm::vec3 cameraDirection = m_ActiveCamera->GetRayDirections()[pixel];

	// Create the ray from the Camera position and direction to pixel
	Ray ray;
	ray.Origin = cameraPosition;
	ray.Direction = cameraDirection;

	// Cast the ray into the scene
	Renderer::HitData hitData = CastRay(ray);
	
	// Default color of the pixel
	glm::vec3 color = glm::vec3(.55f, 0.8f, .50f);

	float y = (hitData.WorldPosition.y - 0.5f) / m_NoiseSettings.Height;

	// No hit color sky
	if (hitData.HitTime < 0.0f)
		color = glm::vec3(0.5f, 0.65f, 1.0f);

	// Color based on noise value
	else if (y < m_NoiseSettings.Water)
		color = glm::vec3(.1f, 0.25f, 1.0f);
	else if (y < m_NoiseSettings.Sand)
		color = glm::vec3(0.7f, 0.7f, 0.3f);
	else if (y > m_NoiseSettings.Snow)
		color = glm::vec3(1.0f, 1.0f, 1.0f);
	else if (y > m_NoiseSettings.Stone)


	return glm::vec4(color, 1.0f);
}

Renderer::HitData Renderer::CastRay(const Ray &ray)
{
	if (m_Settings.Noise && m_Settings.OcTree)
	{
		// Checks all hit octs and returns the minimum time we hit a box of size 1x1x1 that contains a point
		float hitTime = std::numeric_limits<float>::max();
		glm::vec3 hitPoint = glm::vec3(hitTime);
		Utils::ScanChunks(ray, m_ActiveScene->ocTree, hitTime, hitPoint); // hitTime is passed by reference

		if (hitTime == std::numeric_limits<float>::max())
			return Miss(ray);
		
		return ClosestHit(ray, hitTime, hitPoint);
	}
	
	// Do not perform intersection tests if we do not want to render the noise map
	else if (m_Settings.Noise)
	{
		float hitTime = std::numeric_limits<float>::max();
		glm::vec3 hitPoint = glm::vec3(hitTime);
		bool hit = false;

		// Collects all of the 1x1x1 octs containing points in the scene
		std::vector<OcTree*> blocks = {}; 
		m_ActiveScene->ocTree->GetAllChildren(blocks);

		for (const auto &block : blocks)
		{
			if (block->GetSize() > 1)
				continue;

			const AABB &oct = block->GetOct();
			std::vector<glm::vec3> boundaries = oct.GetBoundaries();

			float t = Utils::RayAABBIntersection(ray, boundaries[0], boundaries[1]);

			if (t < 0.0f)
				continue;

			hit = true;
			if (t < hitTime)
			{
				hitPoint = block->GetPoints()[0];
				hitTime = t;
			}
		}
		
		if (!hit)
			return Miss(ray);

		return ClosestHit(ray, hitTime, hitPoint);
	}

	return Miss(ray);
}

Renderer::HitData Renderer::ClosestHit(const Ray &ray, const float &hitTime, const glm::vec3 &hitPoint)
{
	// If we had a hit, return the hit distance and location
	Renderer::HitData hitData;

	hitData.HitTime = hitTime;
	hitData.WorldPosition = hitPoint;

	return hitData;
}

Renderer::HitData Renderer::Miss(const Ray &ray)
{
	// If we did not have a hit, set the hit distance to be negative
	Renderer::HitData hitData;

	hitData.HitTime = -1.0f;

	return hitData;
}
