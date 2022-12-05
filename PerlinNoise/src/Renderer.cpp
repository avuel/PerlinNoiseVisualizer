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
	
	// The Möller–Trumbore intersection algorithm
	// Code adapted from https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	static float RayTriangleIntersect(const Ray &ray, const Triangle &triangle)
	{
		const float EPSILON = 0.0000001;
		glm::vec3 vertex0 = triangle.V0; // Vector3D vertex0 = inTriangle->vertex0;
		glm::vec3 vertex1 = triangle.V1; // Vector3D vertex1 = inTriangle->vertex1;
		glm::vec3 vertex2 = triangle.V2; // Vector3D vertex2 = inTriangle->vertex2;
	
		glm::vec3 edge1, edge2, h, s, q; // Vector3D edge1, edge2, h, s, q;

		float a,f,u,v;
		edge1 = vertex1 - vertex0;
		edge2 = vertex2 - vertex0;
		h = glm::cross(ray.Direction, edge2); // rayVector.crossProduct(edge2);
		a = glm::dot(edge1, h); // edge1.dotProduct(h);
		if (a > -EPSILON && a < EPSILON)
			return -1.0f;    // This ray is parallel to this triangle.
		f = 1.0/a;
		s = ray.Origin - vertex0;
		u = f * (float)glm::dot(s, h);// f *s.dotProduct(h);

		if (u < 0.0 || u > 1.0)
			return -1.0f;

		q = glm::cross(s, edge1); // s.crossProduct(edge1);
		v = f * (float)glm::dot(ray.Direction, q); // rayVector.dotProduct(q);

		if (v < 0.0 || u + v > 1.0)
			return -1.0f;

		// At this stage we can compute t to find out where the intersection point is on the line.
		float t = f * (float)glm::dot(edge2, q); // edge2.dotProduct(q);

		if (t > EPSILON) // ray intersection
		{
			//outIntersectionPoint = rayOrigin + rayVector * t;
			return t;
		}

		else // This means that there is a line intersection but not a ray intersection.
			return -1.0f;
	}

	
	float ScanChunks(const Ray &ray, const std::vector<OcTree*> &chunks)
	{
		float hitDistance = std::numeric_limits<float>::max();
		bool hit = false;

		// Attempt to scan chunks and their insides if we intsect
		// If there is no 1x1x1 box that contains a point and intersects the ray then we do not want to clip our view
		for (auto &chunk : chunks)
		{
			if (chunk->GetPoints().size() == 0)
				continue;

			AABB oct = chunk->GetOct();

			for (int j = 0; j < 12; j++)
			{
				const Triangle &T = oct.Triangles[j];

				float t = RayTriangleIntersect(ray, T);

				if (t >= 0.0f && t < hitDistance)
				{
					if (chunk->GetSize() == 1)
					{
						hit = true;
						hitDistance = t;
					}
					else
					{
						float t2 = ScanChunks(ray, chunk->GetChildren());

						if (t2 > 0.0f && chunk->GetSize() > 1)
						{
							return t2;
						}
					}
				}	
			}
		}

		if (hit)
			return hitDistance;

		return -1.0f;
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

	//delete[] m_NormalBuffer;
	//m_NormalBuffer = new glm::vec3[width * height];
}

void Renderer::Render(Scene &scene, const Camera &camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_ActiveScene->GUI())
	{
		m_NoiseHeight = m_ActiveScene->NoiseHeightScale;
		// Update the noise and get its maximum dimension
		m_ActiveScene->Noise = m_ActiveScene->PerlinNoiseGenerator.GetNoise();
		uint32_t size = std::max((int)std::max(m_ActiveScene->NoiseWidth, m_ActiveScene->NoiseHeight), (int)m_NoiseHeight);

		// Print out the number of data points from the PerlinNoiseGenerator
		std::vector<glm::vec3> points(m_ActiveScene->Noise.size());
		std::cout << "Noise Data Count: " << m_ActiveScene->Noise.size() << '\n';

		// Set the points up for the scene
		for (int i = 0; i < points.size(); i++)
		{
			float x = i % m_ActiveScene->NoiseWidth;
			float y = round(m_ActiveScene->Noise[i] * m_NoiseHeight);
			float z = i / m_ActiveScene->NoiseWidth;
			
			points[i] = glm::vec3(x + 0.5f, y, z + 0.5f);
		}

		m_ActiveScene->OcTree.Generate(size, points);
		std::cout << "Scene Oct Count: " << m_ActiveScene->OcTree.GetOctCount() << '\n';
	}

	const std::vector<glm::vec3> rayDirections = camera.GetRayDirections();
	// https://stackoverflow.com/questions/17694579/use-stdfill-to-populate-vector-with-increasing-numbers
	// Code that creates and fills a vector of size n where the elements are 0,1,2,...,n - 1
	std::vector<int> pixels(m_Width * m_Height); 
	std::iota (std::begin(pixels), std::end(pixels), 0);

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

	Ray ray;
	ray.Origin = cameraPosition;
	ray.Direction = cameraDirection;

	Renderer::HitData hitData = CastRay(ray);
	
	glm::vec3 color = glm::vec3(.55f, 0.8f, .50f);

	float height = m_NoiseHeight;
	float y = hitData.WorldPosition.y / height;

	// No hit color sky
	if (!hitData.Hit)
		color = glm::vec3(0.5f, 0.65f, 1.0f);

	// Color based on noise value
	else if (y < m_Settings.Water)
		color = glm::vec3(.1f, 0.25f, 1.0f);
	else if (y < m_Settings.Sand)
		color = glm::vec3(0.7f, 0.7f, 0.3f);
	else if (y > m_Settings.Snow)
		color = glm::vec3(1.0f, 1.0f, 1.0f);
	else if (y > m_Settings.Stone)
		color = glm::vec3(0.2f, 0.2f, 0.2f);

	return glm::vec4(color, 1.0f);
}

Renderer::HitData Renderer::CastRay(const Ray &ray)
{
	if (m_Settings.Noise && m_Settings.OcTree)
	{
		float hitDistance = Utils::ScanChunks(ray, { &m_ActiveScene->OcTree });
		if (hitDistance == -1.0f)
			return Miss(ray);

		return ClosestHit(ray, hitDistance);
	}
	
	// Do not perform intersection tests if we do not want to render the noise map
	if (m_Settings.Noise && !m_Settings.OcTree)
	{
		float hitDistance = std::numeric_limits<float>::max();
		bool hit = false;
		AABB aabb(glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f));

		for (size_t pixel = 0; pixel < m_ActiveScene->Noise.size(); pixel++)
		{
			float x = pixel % m_ActiveScene->NoiseWidth;
			float y = round(m_NoiseHeight * m_ActiveScene->Noise[pixel]);
			float z = (pixel / m_ActiveScene->NoiseWidth);

			aabb.Translate(glm::vec3(x, y, z));

			for (size_t j = 0; j < 12; j++)
			{
				const Triangle &T = aabb.Triangles[j];

				float t = Utils::RayTriangleIntersect(ray, T);

				if (t >= 0.0f && t < hitDistance)
				{
					hitDistance = t;
					hit = true;
				}
			}
			aabb.Translate(glm::vec3(-x, -y, -z));
		}
		if (!hit)
			return Miss(ray);

		return ClosestHit(ray, hitDistance);
	}

	return Miss(ray);
}

Renderer::HitData Renderer::ClosestHit(const Ray &ray, const float &hitDistance)
{
	Renderer::HitData hitData;

	hitData.HitDistance = hitDistance;
	hitData.WorldPosition = ray.Origin + ray.Direction * hitDistance;
	//hitData.WorldNormal = hitNormal;
	hitData.Hit = true;

	return hitData;
}

Renderer::HitData Renderer::Miss(const Ray &ray)
{
	Renderer::HitData hitData;

	hitData.HitDistance = -1.0f;
	//hitData.WorldNormal = glm::vec3(0.0f, 0.0f, 0.0f);
	hitData.Hit = false;

	return hitData;
}
