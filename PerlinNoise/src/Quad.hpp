#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Quad
{
	glm::vec3 Vertices[4] = {};
	glm::vec2 Boundaries[3] = {};
	glm::vec3 Normal{ 0.0f };

	Quad() = default;
	
	Quad(const std::vector<glm::vec3> &verts, const std::vector<glm::vec2> &bounds, const glm::vec3 &normal)
	{
		Vertices[0] = verts[0];
		Vertices[1] = verts[1];
		Vertices[2] = verts[2];
		Vertices[3] = verts[3];

		Boundaries[0] = bounds[0];
		Boundaries[1] = bounds[1];
		Boundaries[2] = bounds[2];

		Normal = normal;
	}

	bool XYCointainsPoint(const glm::vec3 &point) const
	{
		float xMin = Boundaries[0][0];
		float xMax = Boundaries[0][1];
		float yMin = Boundaries[1][0];
		float yMax = Boundaries[1][1];

		return ((point.x <= xMax) && (point.x > xMin) &&
			    (point.y <= yMax) && (point.y > yMin));
	}

	bool XZContainsPoint(const glm::vec3 &point) const
	{
		float xMin = Boundaries[0][0];
		float xMax = Boundaries[0][1];
		float zMin = Boundaries[2][0];
		float zMax = Boundaries[2][1];

		return ((point.x <= xMax) && (point.x > xMin) &&
			    (point.z <= zMax) && (point.z > zMin));
	}

	bool YZContainsPoint(const glm::vec3 &point) const
	{
		float yMin = Boundaries[1][0];
		float yMax = Boundaries[1][1];
		float zMin = Boundaries[2][0];
		float zMax = Boundaries[2][1];

		return ((point.y <= yMax) && (point.y > yMin) &&
			    (point.z <= zMax) && (point.z > zMin));
	}
};