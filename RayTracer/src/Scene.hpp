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
	int Shinyness = 4.0f;
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
	int MaterialIndex = 0;
};

struct Light
{
	glm::vec3 Color{ 0.0f };

	Light(const glm::vec3 &color) : Color(color) {}
	virtual glm::vec3 GetDirection(const glm::vec3 &point) { return glm::vec3(0.0f); };
};

struct DirectionalLight : Light
{
	glm::vec3 Direction;

	DirectionalLight(const glm::vec3 &direction, const glm::vec3 &color) : Light(color), Direction(glm::normalize(direction)) { }
	glm::vec3 GetDirection(const glm::vec3 &point)
	{
		return Direction;
	}
};

struct PointLight : Light
{
	glm::vec3 Origin;

	PointLight(const glm::vec3 &origin, const glm::vec3 &color) : Light(color), Origin(origin) { }
	glm::vec3 GetDirection(const glm::vec3 &point)
	{
		return glm::normalize(point - Origin);
	}
};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Cube> Cubes;
	std::vector<Material> Materials;
	std::vector<std::unique_ptr<Light>> Lights;
};