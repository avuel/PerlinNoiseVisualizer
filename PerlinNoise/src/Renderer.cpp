#include "Renderer.hpp"
#include "Walnut/Random.h"

#include <numeric>
#include <algorithm>
#include <execution>

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

	delete[] m_NormalBuffer;
	m_NormalBuffer = new glm::vec3[width * height];
}

void Renderer::Render(Scene &scene, const Camera &camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_ActiveScene->GUI())
		m_ActiveScene->Noise = m_ActiveScene->PerlinNoiseGenerator.GetNoise();

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

		/*
		std::for_each(std::execution::par, pixels.begin(), pixels.end(), [this](const int &pixel)
			{
				uint32_t x = pixel % m_Width;
				uint32_t y = pixel / m_Width;

				glm::vec3 normal = m_NormalBuffer[x + y * m_Width];

				bool altered = false;

				uint32_t borderColor = Utils::ConvertToRGBA(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));

				for (int j = -1; j < 2; j++)
				{
					uint32_t y0 = y + j;

					if (y0 >= m_Height)
						continue;

					for (int i = -1; i < 2; i++)
					{
						uint32_t x0 = x + j;
						
						if (x0 >= m_Width)
							continue;

						if (normal != m_NormalBuffer[x0 + y0 * m_Width])
						{
							//if (m_ImageData[x0 + y0 * m_Width] != borderColor)
							m_ColorBuffer[x + y * m_Width] = borderColor;
							altered = true;
							break;
						}
					}

					if (altered)
						break;
				}
			});
		*/
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

		for (uint32_t y = 0; y < m_Height; y++)
		{
			for (uint32_t x = 0; x < m_Width; x++)
			{
				glm::vec3 normal = m_NormalBuffer[x + y * m_Width];

				bool altered = false;

				uint32_t borderColor = Utils::ConvertToRGBA(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));

				for (int j = -1; j < 2; j++)
				{
					uint32_t y0 = y + j;

					if (y0 >= m_Height)
						continue;

					for (int i = -1; i < 2; i++)
					{
						uint32_t x0 = x + j;

						if (x0 >= m_Width)
							continue;

						if (normal != m_NormalBuffer[x0 + y0 * m_Width])
						{
							//if (m_ImageData[x0 + y0 * m_Width] != borderColor)
							m_ColorBuffer[x + y * m_Width] = borderColor;
							altered = true;
							break;
						}
					}

					if (altered)
						break;
				}
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
	//m_NormalBuffer[x + y * m_Width] = hitData.WorldNormal;
	
	glm::vec3 color = glm::vec3(.55f, 0.8f, .50f);

	// No hit color sky
	if (hitData.HitDistance < 0.0f)
		color = glm::vec3(0.5f, 0.65f, 1.0f);

	return glm::vec4(color, 1.0f);

	// If we are in the bottom 20%, color blue for water
	if (m_ActiveScene->Noise[hitData.HitPixel] < 0.0125f)
		color = glm::vec3(.1f, 0.25f, 1.0f);

	else if (m_ActiveScene->Noise[hitData.HitPixel] < 0.05f)
		color = glm::vec3(0.7f, 0.7f, 0.3f);

	else if (m_ActiveScene->Noise[hitData.HitPixel] <= 0.3f && m_ActiveScene->Noise[hitData.HitPixel] > 0.25f)
		color = glm::vec3(0.2f, 0.2f, 0.2f);

	else if (m_ActiveScene->Noise[hitData.HitPixel] > 0.3f)
		color = glm::vec3(1.0f, 1.0f, 1.0f);


	return glm::vec4(color, 1.0f);
}

Renderer::HitData Renderer::CastRay(const Ray &ray)
{
	float hitDistance = std::numeric_limits<float>::max(); // Maximum Float Value
	size_t hitFace = -1;
	size_t hitIndex = -1;
	size_t hitPixel = -1;
	glm::vec3 hitNormal {0.0f};
	
	if (m_Settings.Clip)
	{
		const AABB &boundingBox = m_ActiveScene->BoundingBox;

		for (size_t j = 0; j < 12; j++)
		{
			const Triangle &T = boundingBox.Triangles[j];

			float t = Utils::RayTriangleIntersect(ray, T);

			if (m_Settings.BoundingBox)
			{
				if (t >= 0.0f && t < hitDistance)
				{
					hitDistance = t;
					hitFace = j / 2;
					hitIndex = -2;
					hitNormal = boundingBox.Normals[hitFace];
				}

				if (j == 11)
				{
					//return ClosestHit(ray, hitDistance, hitNormal);
				}
			}

			else
			{
				if (t >= 0.0f)
					break;

				if (j == 11)
				{
					return Miss(ray);
				}
			}
		}
	}
	
	// Do not perform intersection tests if we do not want to render the noise map
	if (m_Settings.Noise)
	{
		/*
		size_t octWidth = m_ActiveScene->NoiseWidth;
		size_t octHeight = m_ActiveScene->NoiseHeightScale;
		size_t octDepth = m_ActiveScene->NoiseHeight;

		int xDims = 0;
		int yDims = 0;
		int zDims = 0;

		int majorAxis = std::max(std::max(octWidth, octHeight), octDepth);
		int pow = 1;

		while (majorAxis / pow > 1)
		{
			pow *= 2;

			if (octWidth / pow > 1)
				xDims++;

			if (octHeight / pow > 1)
				yDims++;

			if (octDepth / pow > 1)
				zDims++;
		}

		int dim = std::max(std::max(xDims, yDims), zDims);

		glm::vec3 octOrigin = glm::vec3((float)(octWidth - 1) / 2.0f, (float)(octHeight - 1) / 2.0f, (float)(octDepth - 1) / 2.0f);
		//uint8_t octIndex = 0;

		pow = 1;

		for (size_t i = 0; i < dim; i++)
		{
			float width  = octWidth  / 2.0f;//(xDims == 1) ? octWidth  : octWidth  / 4.0f;
			float height = octHeight / 2.0f;//(yDims == 1) ? octHeight : octHeight / 4.0f;
			float depth  = octDepth  / 2.0f;//(zDims == 1) ? octDepth  : octDepth  / 4.0f;

			if (octWidth / pow <= 1) xDims = 1;
			if (octHeight / pow <= 1) yDims = 1;
			if (octDepth / pow <= 1) zDims = 1;

			AABB octs(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(width, height, depth));

			for (uint8_t oct = 0; oct < (xDims * yDims * zDims); oct++)
			{
				uint8_t xDim = oct & 1;
				uint8_t yDim = (oct & 4) >> 2;
				uint8_t zDim = (oct & 8) >> 4;

				float x = (xDims == 1) ? octOrigin.x : octOrigin.x - (width / 2.0f)  + xDim * width;
				float y = (yDims == 1) ? octOrigin.y : octOrigin.y - (height / 2.0f)  + yDim * octHeight;
				float z = (zDims == 1) ? octOrigin.z : octOrigin.z - (depth / 2.0f)  + zDim * octDepth;

				octs.Translate(glm::vec3(x,y,z));

				for (size_t j = 0; j < 12; j++)
				{
					const Triangle &T = octs.Triangles[j];

					float t = Utils::RayTriangleIntersect(ray, T);

					if (t >= 0.0f && t < hitDistance)
					{
						hitDistance = t;
						hitFace = j / 2;
						hitNormal = octs.Normals[hitFace];
					}
				}
			}

			pow *= 2;
		}
		*/

		
		for (size_t pixel = 0; pixel < m_ActiveScene->Noise.size(); pixel++)
		{
			float x = pixel % m_ActiveScene->NoiseWidth;
			float y = m_ActiveScene->NoiseHeightScale * m_ActiveScene->Noise[pixel];
			float z = (float)(pixel / m_ActiveScene->NoiseWidth);

			for (size_t i = 0; i < m_ActiveScene->AABBs.size(); i++)
			{
				AABB aabb = m_ActiveScene->AABBs[i];
				aabb.Translate(glm::vec3(x, y, z));

				for (size_t j = 0; j < 12; j++)
				{
				const Triangle &T = aabb.Triangles[j];

				float t = Utils::RayTriangleIntersect(ray, T);

					if (t >= 0.0f && t < hitDistance)
					{
						hitDistance = t;
						hitIndex = i;
						hitFace = j / 2;
						hitNormal = m_ActiveScene->AABBs[hitIndex].Normals[hitFace];
						hitPixel = pixel;
					}
				}
			}
		}
	}

	

	if (hitIndex == -1)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, hitNormal, hitPixel);
}

Renderer::HitData Renderer::ClosestHit(const Ray &ray, const float &hitDistance, const glm::vec3 &hitNormal, const size_t &hitPixel)
{
	Renderer::HitData hitData;

	hitData.HitDistance = hitDistance;
	hitData.WorldPosition = ray.Origin + ray.Direction * hitDistance;
	hitData.WorldNormal = hitNormal;
	hitData.HitPixel = hitPixel;

	return hitData;
}

Renderer::HitData Renderer::Miss(const Ray &ray)
{
	Renderer::HitData hitData;

	hitData.HitDistance = -1.0f;
	hitData.WorldNormal = glm::vec3(0.0f, 0.0f, 0.0f);

	return hitData;
}
