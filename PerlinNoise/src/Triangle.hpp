#pragma once

#include <glm/glm.hpp>

struct Triangle
{
	glm::vec3 V0{ 0.0f };
	glm::vec3 V1{ 0.0f };
	glm::vec3 V2{ 0.0f };

	Triangle() = default;

	Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
	{
		V0 = v0;
		V1 = v1;
		V2 = V2;
	}
};