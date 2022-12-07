#pragma once

#include <glm/glm.hpp>

struct Ray
{
	glm::vec3 Origin;
	glm::vec3 Direction;

	Ray() = default;

	Ray(const glm::vec3 &origin, const glm::vec3 &direction)
	{
		Origin = origin;
		Direction = direction;
	}
};