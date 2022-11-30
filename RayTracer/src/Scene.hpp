#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <iostream>

enum ObjectTypes : uint8_t
{
	Sphere_t = 0,
	Cube_t
};

struct Material
{
	glm::vec3 Albedo{ 0.0f };
	float Reflectiveness = 0.0f;
	int Shinyness = 4;
};

struct Sphere
{
	glm::vec3 Origin{ 0.0f };
	float Radius = 0.5f;

	int MaterialIndex = 0;
};

struct Triangle
{
	glm::vec3 V0;
	glm::vec3 V1;
	glm::vec3 V2;
};

struct Quad
{
	glm::vec3 Normal{ 0.0f };
	Triangle Tris[2] = {};
};

struct Cube
{
	Triangle Triangles[12] = {};
	glm::vec3 Normals[12] = {};
	glm::vec3 Origin{ 0.0f };
	int MaterialIndex = 0;

	void Translate(const glm::vec3 &translation)
	{
		for (auto &triangle : Triangles)
		{
			triangle.V0 = glm::vec3(triangle.V0.x + translation.x, triangle.V0.y + translation.y, triangle.V0.z + translation.z);
			triangle.V1 = glm::vec3(triangle.V1.x + translation.x, triangle.V1.y + translation.y, triangle.V1.z + translation.z);
			triangle.V2 = glm::vec3(triangle.V2.x + translation.x, triangle.V2.y + translation.y, triangle.V2.z + translation.z);
		}
	}
};

struct Light
{
	glm::vec3 Color{ 0.0f };

	Light(const glm::vec3 &color) : Color(color) {}
	virtual glm::vec3 GetDirection(const glm::vec3 &point) { return glm::vec3(0.0f); };
	virtual const char* GetType() { return "0"; };
	virtual glm::vec3* GetData() { return nullptr; };
};

struct DirectionalLight : Light
{
	glm::vec3 Direction;

	DirectionalLight(const glm::vec3 &direction, const glm::vec3 &color) : Light(color), Direction(direction) { }
	glm::vec3 GetDirection(const glm::vec3 &point)
	{
		return glm::normalize(Direction);
	}
	glm::vec3 *GetData() { return &Direction; };
	const char* GetType() { return "Direction"; };
};

struct PointLight : Light
{
	glm::vec3 Origin;

	PointLight(const glm::vec3 &origin, const glm::vec3 &color) : Light(color), Origin(origin) { }
	glm::vec3 GetDirection(const glm::vec3 &point)
	{
		return glm::normalize(point - Origin);
	}
	glm::vec3* GetData() { return &Origin; };
	const char* GetType() { return "Origin"; };

};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Cube> Cubes;
	std::vector<const char *> ObjectNames;
	std::vector<Material> Materials;
	std::vector<std::unique_ptr<Light>> Lights;
};