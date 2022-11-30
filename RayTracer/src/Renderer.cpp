#include "Renderer.hpp"
#include "Walnut/Random.h"

#include <iostream>

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


	static float luminance(glm::vec3 color)
	{
		// Return the luminance of the color
		return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
	}

	static glm::vec3 gamma(glm::vec3 color)
	{
		// sRGB gamma correction
		float exp = 1.0/2.4;
		if(color.r < 0.0031308)
			color.r *= 12.92;
		else
			color.r = 1.055 * glm::pow(color.r, exp) - 0.055;
		if(color.g < 0.0031308)
			color.g *= 12.92;
		else
			color.g = 1.055 * glm::pow(color.g, exp) - 0.055;
		if(color.b < 0.0031308)
			color.b *= 12.92;
		else
			color.b = 1.055 * glm::pow(color.b, exp) - 0.055;
		return color;
	}

	static glm::vec3 PhongLighting(const glm::vec3 &lightDir, const glm::vec3 &normal, const glm::vec3 &cameraDir, 
									const glm::vec3 &objectColor, const glm::vec3 &lightColor, const float &specularExponent)
	{
		float diffuseIntensity = glm::max(glm::dot(-lightDir, normal), 0.0f);

		float specularIntensity = glm::step(0.0f, glm::dot(-lightDir, normal)) // check direction of light
			* glm::pow(glm::max(glm::dot(glm::reflect(-lightDir, normal), glm::normalize(cameraDir)), 0.0f), specularExponent);


		glm::vec3 ambient = 0.2f * objectColor;
		glm::vec3 diffuse = objectColor * lightColor * diffuseIntensity;
		glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f) * lightColor * specularIntensity;

		// Return the tone mapped color
		return (gamma((ambient + diffuse + specular) / (1.0f + luminance(ambient + diffuse + specular))));
	}
	
	static float RaySphereIntersect(const Ray &ray, const Sphere &sphere)
	{
		glm::vec3 origin = ray.Origin - sphere.Origin;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - a * c;

		if (discriminant < 0.0f)
			return -1.0f;

		float sqrt_discriminant = glm::sqrt(discriminant); // Only take the square root once

		float t0 = (-b - sqrt_discriminant) / a;
		float t1 = (-b + sqrt_discriminant) / a;

		if (t0 < 0.0f)
			return t1;
		return t0;
	}
	
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

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render(const Scene &scene, const Camera &camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	const std::vector<glm::vec3> rayDirections = camera.GetRayDirections();
	// https://stackoverflow.com/questions/17694579/use-stdfill-to-populate-vector-with-increasing-numbers
	// Code that creates and fills a vector of size n where the elements are 0,1,2,...,n
	std::vector<int> pixels(m_FinalImage->GetHeight() * m_FinalImage->GetWidth()); 
	std::iota (std::begin(pixels), std::end(pixels), 0);


	if (m_Settings.Parallel) 
	{
		std::for_each(std::execution::par, pixels.begin(), pixels.end(), [this](int &i)
			{
				const uint32_t x = i % m_FinalImage->GetWidth();
				const uint32_t y = i / m_FinalImage->GetWidth();
				glm::vec4 color = PerPixel(x, y);

				color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
				m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
			});
	}

	else
	{
		for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
		{
			for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
			{
				glm::vec4 color = PerPixel(x, y);

				color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
				m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
			}
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

std::shared_ptr<Walnut::Image> Renderer::GetFinalImage()
{
	return m_FinalImage;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	glm::vec3 cameraPosition = m_ActiveCamera->GetPosition();
	glm::vec3 cameraDirection = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	Ray ray;
	ray.Origin = cameraPosition;
	ray.Direction = cameraDirection;

	bool shadow = false;
	const int bounces = 2;

	std::vector<Renderer::HitData> hits;

	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitData hitData = TraceRay(ray);
		hits.push_back(hitData);

		// If we do not have an intersection, reflect the sky
		if (hitData.HitDistance < 0.0f)
		{	
			break;
		}

		// Setup the bounced ray for the next loop
		ray.Origin = hitData.WorldPosition + hitData.WorldNormal * 0.0001f; // Increment the origin along the normal slightly otherwise it
		ray.Direction = glm::reflect(ray.Direction, hitData.WorldNormal);   // will say it intersects the object it bounced off
	}

	std::vector<glm::vec3> localColors;
	std::vector<float> intensities;
	
	// Loop over the hit objects recursively and perform the local lighting model
	for (auto it = std::rbegin(hits); it != std::rend(hits); it++)
	{
		Renderer::HitData hitData = *it;

		intensities.push_back(hitData.LightMultiplier);
	
		if (hitData.HitDistance < 0.0f)
		{
			// Add the ambient sky color if we did not hit an object
			localColors.push_back(m_ActiveScene->Materials[0].Albedo);
			continue;
		}

		Ray shadowRay;
		bool inShadow = true;

		for (auto const &light : m_ActiveScene->Lights)
		{
			shadowRay.Origin = hitData.WorldPosition + hitData.WorldNormal * 0.0001f;
			shadowRay.Direction = -(light->GetDirection(shadowRay.Origin));

			Renderer::HitData shadowHit = TraceRay(shadowRay);

			if (shadowHit.HitDistance >= 0.0f)
			{
				// Add only the ambient color if we are in shadow (not visible by light)
				//localColors.push_back(0.2f * hitData.Color);
				continue;
			}

			inShadow = false;
			break;
		}
		if (inShadow)
		{
			localColors.push_back(0.2f * hitData.Color);
			continue;
		}
		glm::vec3 color{ 0.0f };
		for (auto const &light: m_ActiveScene->Lights)
		{
			shadowRay.Origin = hitData.WorldPosition + hitData.WorldNormal * 0.0001f;
			shadowRay.Direction = -(light->GetDirection(shadowRay.Origin));

			// Tone mapped color for the local model
			color += Utils::PhongLighting(-shadowRay.Direction, hitData.WorldNormal, cameraDirection,
				hitData.Color, light->Color, hitData.SpecularExponent);
		}
		localColors.push_back(color);
	}

	// Add the colors and multiply by the light intensity reflected from the object we bounced off previously
	glm::vec3 color{ 0.0f };

	for (size_t i = 0; i < localColors.size(); i++)
	{
		if (i == localColors.size() - 1)
			color += localColors[i]; // If we are the first object we hit, it receives full light intensity in our model
		else
			color += localColors[i] * intensities[i + 1];
	}
	
	return glm::vec4(color, 1.0f);
}

Renderer::HitData Renderer::TraceRay(const Ray &ray)
{
	int closestObject = -1; // If we do not hit a sphere it stays nullptr
	uint8_t objectType = -1; // Track which type of object we hit
	uint8_t objectFace = -1; // Track which face of an object we hit
	float hitDistance = std::numeric_limits<float>::max(); // Maximum Float Value

	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere &sphere = m_ActiveScene->Spheres[i];

		float t = Utils::RaySphereIntersect(ray, sphere);
		
		if (t >= 0.0f && t < hitDistance)
		{
			hitDistance = t;
			closestObject = i;
			objectType = Sphere_t;
		}
	}
	
	for (size_t i = 0; i < m_ActiveScene->Cubes.size(); i++)
	{
		const Cube &cube = m_ActiveScene->Cubes[i];

		for (size_t j = 0; j < 12; j++)
		{
			const Triangle &T = cube.Triangles[j];

			float t = Utils::RayTriangleIntersect(ray, T);
			
			if (t >= 0.0f && t < hitDistance)
			{
				hitDistance = t;
				closestObject = i;
				objectType = Cube_t;
				objectFace = j;
			}
		}
	}

	if (closestObject == -1)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestObject, objectType, objectFace);
}

Renderer::HitData Renderer::ClosestHit(const Ray &ray, float hitDistance, int objectIndex, uint8_t objectType, uint8_t objectFace)
{
	Renderer::HitData hitData;
	hitData.HitDistance = hitDistance;
	hitData.ObjectIndex = objectIndex;
	hitData.ObjectType = objectType;
	

	if (objectType == Sphere_t)
	{
		const Sphere &closestSphere = m_ActiveScene->Spheres[objectIndex];

		hitData.WorldPosition = ray.Origin + hitDistance * ray.Direction;
		hitData.WorldNormal = glm::normalize(hitData.WorldPosition - closestSphere.Origin);

		const Material &material = m_ActiveScene->Materials[m_ActiveScene->Spheres[hitData.ObjectIndex].MaterialIndex];

		hitData.LightMultiplier = material.Reflectiveness;
		hitData.Color = material.Albedo;
		hitData.SpecularExponent = (float)material.Shinyness;
	}
	
	if (objectType == Cube_t)
	{
		const Cube &closestCube = m_ActiveScene->Cubes[objectIndex];

		hitData.WorldPosition = ray.Origin + ray.Direction * hitDistance;
		hitData.WorldNormal = closestCube.Normals[objectFace];

		const Material &material = m_ActiveScene->Materials[m_ActiveScene->Cubes[hitData.ObjectIndex].MaterialIndex];

		hitData.LightMultiplier = material.Reflectiveness;
		hitData.Color = material.Albedo;
		hitData.SpecularExponent = (float)material.Shinyness;
	}
	
	return hitData;
}

Renderer::HitData Renderer::Miss(const Ray &ray)
{
	Renderer::HitData hitData;
	hitData.HitDistance = -1.0f;

	return hitData;
}
