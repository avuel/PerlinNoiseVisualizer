#pragma once

#include "Triangle.hpp"
#include "Quad.hpp"

struct AABB
{
	Triangle Triangles[12] = {};
	Quad Faces[6];
	glm::vec3 Normals[6] = { {0,0,-1}, {1,0,0}, {0,0,1}, {-1,0,0}, {0,1,0}, {0,-1,0} };
	glm::vec3 Vertices[8] = {};
	glm::vec3 Origin{ 0.0f };
	glm::vec3 Dims{ 0.0f };

	AABB() = default;

	AABB(const glm::vec3 &origin, const glm::vec3 &dims)
	{
		Origin = origin;
		Dims = dims;
		Vertices[0] = glm::vec3(origin.x - dims.x, origin.y - dims.y, origin.z - dims.z); // (-, -, -)
		Vertices[1] = glm::vec3(origin.x + dims.x, origin.y - dims.y, origin.z - dims.z); // (+, -, -)
		Vertices[2] = glm::vec3(origin.x - dims.x, origin.y + dims.y, origin.z - dims.z); // (-, +, -)
		Vertices[3] = glm::vec3(origin.x + dims.x, origin.y + dims.y, origin.z - dims.z); // (+, +, -)
		Vertices[4] = glm::vec3(origin.x + dims.x, origin.y - dims.y, origin.z + dims.z); // (+, -, +)
		Vertices[5] = glm::vec3(origin.x + dims.x, origin.y + dims.y, origin.z + dims.z); // (+, +, +)
		Vertices[6] = glm::vec3(origin.x - dims.x, origin.y - dims.y, origin.z + dims.z); // (-, -, +)
		Vertices[7] = glm::vec3(origin.x - dims.x, origin.y + dims.y, origin.z + dims.z); // (-, +, +)

		glm::vec2 xBounds = glm::vec2(Vertices[0].x, Vertices[5].x);
		glm::vec2 yBounds = glm::vec2(Vertices[0].y, Vertices[5].y);
		glm::vec2 zBounds = glm::vec2(Vertices[0].z, Vertices[5].z);

		Faces[0] = Quad({ Vertices[0], Vertices[1], Vertices[3], Vertices[2] }, { xBounds, yBounds, zBounds }, Normals[0]); // Front Face
		Faces[1] = Quad({ Vertices[1], Vertices[4], Vertices[5], Vertices[3] }, { xBounds, yBounds, zBounds }, Normals[1]); // Right Face
		Faces[2] = Quad({ Vertices[4], Vertices[6], Vertices[7], Vertices[5] }, { xBounds, yBounds, zBounds }, Normals[2]); // Back Face
		Faces[3] = Quad({ Vertices[6], Vertices[0], Vertices[2], Vertices[7] }, { xBounds, yBounds, zBounds }, Normals[3]); // Left Face
		Faces[4] = Quad({ Vertices[2], Vertices[3], Vertices[5], Vertices[7] }, { xBounds, yBounds, zBounds }, Normals[4]); // Up Face
		Faces[5] = Quad({ Vertices[4], Vertices[1], Vertices[0], Vertices[6] }, { xBounds, yBounds, zBounds }, Normals[5]); // Down Face

		// Front Face
		Triangle triangle0;
		triangle0.V0 = Vertices[0]; // (-, -, -)
		triangle0.V1 = Vertices[1]; // (+, -, -)
		triangle0.V2 = Vertices[2]; // (-, +, -)
		Triangle triangle1;
		triangle1.V0 = Vertices[1]; // (+, -, -)
		triangle1.V1 = Vertices[3]; // (+, +, -)
		triangle1.V2 = Vertices[2]; // (-, +, -)

		// Right Face
		Triangle triangle2;
		triangle2.V0 = Vertices[1]; // (+, -, -)
		triangle2.V1 = Vertices[4]; // (+, -, +)
		triangle2.V2 = Vertices[3]; // (+, +, -)
		Triangle triangle3;
		triangle3.V0 = Vertices[4]; // (+, -, +)
		triangle3.V1 = Vertices[5]; // (+, +, +)
		triangle3.V2 = Vertices[3]; // (+, +, -)

		// Back Face
		Triangle triangle4;
		triangle4.V0 = Vertices[4]; // (+, -, +)
		triangle4.V1 = Vertices[6]; // (-, -, +)
		triangle4.V2 = Vertices[5]; // (+, +, +)
		Triangle triangle5;
		triangle5.V0 = Vertices[6]; // (-, -, +)
		triangle5.V1 = Vertices[7]; // (-, +, +)
		triangle5.V2 = Vertices[5]; // (+, +, +)

		// Left Face
		Triangle triangle6;
		triangle6.V0 = Vertices[6]; // (-, -, +)
		triangle6.V1 = Vertices[0]; // (-, -, -)
		triangle6.V2 = Vertices[7]; // (-, +, +)
		Triangle triangle7;
		triangle7.V0 = Vertices[0]; // (-, -, -)
		triangle7.V1 = Vertices[2]; // (-, +, -)
		triangle7.V2 = Vertices[7]; // (-, +, +)

		// Top Face
		Triangle triangle8;
		triangle8.V0 = Vertices[2]; // (-, +, -)
		triangle8.V1 = Vertices[3]; // (+, +, -)
		triangle8.V2 = Vertices[7]; // (-, +, +)
		Triangle triangle9;
		triangle9.V0 = Vertices[3]; // (+, +, -)
		triangle9.V1 = Vertices[5]; // (+, +, +)
		triangle9.V2 = Vertices[7]; // (-, +, +)

		// Bottom Face
		Triangle triangle10;
		triangle10.V0 = Vertices[0]; // (-, -, -)
		triangle10.V1 = Vertices[6]; // (-, -, +)
		triangle10.V2 = Vertices[1]; // (+, -, -)
		Triangle triangle11;
		triangle11.V0 = Vertices[6]; // (-, -, +)
		triangle11.V1 = Vertices[4]; // (+, -, +)
		triangle11.V2 = Vertices[1]; // (+, -, -)

		Triangles[0] = triangle0;
		Triangles[1] = triangle1;
		Triangles[2] = triangle2;
		Triangles[3] = triangle3;
		Triangles[4] = triangle4;
		Triangles[5] = triangle5;
		Triangles[6] = triangle6;
		Triangles[7] = triangle7;
		Triangles[8] = triangle8;
		Triangles[9] = triangle9;
		Triangles[10] = triangle10;
		Triangles[11] = triangle11;
	}

	void Translate(const glm::vec3 &translation)
	{
		for (auto &triangle : Triangles)
		{
			triangle.V0 = glm::vec3(triangle.V0.x + translation.x, triangle.V0.y + translation.y, triangle.V0.z + translation.z);
			triangle.V1 = glm::vec3(triangle.V1.x + translation.x, triangle.V1.y + translation.y, triangle.V1.z + translation.z);
			triangle.V2 = glm::vec3(triangle.V2.x + translation.x, triangle.V2.y + translation.y, triangle.V2.z + translation.z);
		}
	}

	bool ContainsPoint(const glm::vec3 &point) const
	{
		return ((point.x <= (Origin.x + Dims.x)) && (point.x > (Origin.x - Dims.x)) &&
			    (point.y <= (Origin.y + Dims.y)) && (point.y > (Origin.y - Dims.y)) &&
			    (point.z <= (Origin.z + Dims.z)) && (point.z > (Origin.z - Dims.z)));
	}

	std::vector<glm::vec3> GetBoundaries() const
	{
		return { Vertices[0], Vertices[5] };
	}

	void operator= (const AABB &right)
	{
		Origin = right.Origin;
		Dims = right.Dims;

		for (int i = 0; i < 12; i++)
		{
			Triangles[i] = right.Triangles[i];
		}

		for (int i = 0; i < 6; i++)
		{
			Faces[i] = right.Faces[i];
		}

		for (int i = 0; i < 8; i++)
		{
			Vertices[i] = right.Vertices[i];
		}
	}
};