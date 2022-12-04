#pragma once

#include "Triangle.hpp"

struct AABB
{
	Triangle Triangles[12] = {};
	glm::vec3 Normals[6] = {};
	glm::vec3 Origin{ 0.0f };

	AABB() = default;

	AABB(const glm::vec3 &origin, const glm::vec3 &dims)
	{
		// Back Face
		Triangle triangle0;
		triangle0.V0 = { origin.x - dims.x, origin.y - dims.y, origin.z - dims.z }; // (-, -, -)
		triangle0.V1 = { origin.x + dims.x, origin.y - dims.y, origin.z - dims.z }; // (+, -, -)
		triangle0.V2 = { origin.x - dims.x, origin.y + dims.y, origin.z - dims.z }; // (-, +, -)
		Triangle triangle1;
		triangle1.V0 = { origin.x + dims.x, origin.y - dims.y, origin.z - dims.z }; // (+, -, -)
		triangle1.V1 = { origin.x + dims.x, origin.y + dims.y, origin.z - dims.z }; // (+, +, -)
		triangle1.V2 = { origin.x - dims.x, origin.y + dims.y, origin.z - dims.z }; // (-, +, -)

																					// Right Face
		Triangle triangle2;
		triangle2.V0 = { origin.x + dims.x, origin.y - dims.y, origin.z - dims.z }; // (+, -, -)
		triangle2.V1 = { origin.x + dims.x, origin.y - dims.y, origin.z + dims.z }; // (+, -, +)
		triangle2.V2 = { origin.x + dims.x, origin.y + dims.y, origin.z - dims.z }; // (+, +, -)
		Triangle triangle3;
		triangle3.V0 = { origin.x + dims.x, origin.y - dims.y, origin.z + dims.z }; // (+, -, +)
		triangle3.V1 = { origin.x + dims.x, origin.y + dims.y, origin.z + dims.z }; // (+, +, +)
		triangle3.V2 = { origin.x + dims.x, origin.y + dims.y, origin.z - dims.z }; // (+, +, -)

																					// Front Face
		Triangle triangle4;
		triangle4.V0 = { origin.x + dims.x, origin.y - dims.y, origin.z + dims.z }; // (+, -, +)
		triangle4.V1 = { origin.x - dims.x, origin.y - dims.y, origin.z + dims.z }; // (-, -, +)
		triangle4.V2 = { origin.x + dims.x, origin.y + dims.y, origin.z + dims.z }; // (+, +, +)
		Triangle triangle5;
		triangle5.V0 = { origin.x - dims.x, origin.y - dims.y, origin.z + dims.z }; // (-, -, +)
		triangle5.V1 = { origin.x - dims.x, origin.y + dims.y, origin.z + dims.z }; // (-, +, +)
		triangle5.V2 = { origin.x + dims.x, origin.y + dims.y, origin.z + dims.z }; // (+, +, +)

																					// Left Face
		Triangle triangle6;
		triangle6.V0 = { origin.x - dims.x, origin.y - dims.y, origin.z + dims.z }; // (-, -, +)
		triangle6.V1 = { origin.x - dims.x, origin.y - dims.y, origin.z - dims.z }; // (-, -, -)
		triangle6.V2 = { origin.x - dims.x, origin.y + dims.y, origin.z + dims.z }; // (-, +, +)
		Triangle triangle7;
		triangle7.V0 = { origin.x - dims.x, origin.y - dims.y, origin.z - dims.z }; // (-, -, -)
		triangle7.V1 = { origin.x - dims.x, origin.y + dims.y, origin.z - dims.z }; // (-, +, -)
		triangle7.V2 = { origin.x - dims.x, origin.y + dims.y, origin.z + dims.z }; // (-, +, +)

																					// Top Face
		Triangle triangle8;
		triangle8.V0 = { origin.x - dims.x, origin.y + dims.y, origin.z - dims.z }; // (-, +, -)
		triangle8.V1 = { origin.x + dims.x, origin.y + dims.y, origin.z - dims.z }; // (+, +, -)
		triangle8.V2 = { origin.x - dims.x, origin.y + dims.y, origin.z + dims.z }; // (-, +, +)
		Triangle triangle9;
		triangle9.V0 = { origin.x + dims.x, origin.y + dims.y, origin.z - dims.z }; // (+, +, -)
		triangle9.V1 = { origin.x + dims.x, origin.y + dims.y, origin.z + dims.z }; // (+, +, +)
		triangle9.V2 = { origin.x - dims.x, origin.y + dims.y, origin.z + dims.z }; // (-, +, +)

																					// Bottom Face
		Triangle triangle10;
		triangle10.V0 = { origin.x - dims.x, origin.y - dims.y, origin.z - dims.z }; // (-, -, -)
		triangle10.V1 = { origin.x - dims.x, origin.y - dims.y, origin.z + dims.z }; // (-, -, +)
		triangle10.V2 = { origin.x + dims.x, origin.y - dims.y, origin.z - dims.z }; // (+, -, -)
		Triangle triangle11;
		triangle11.V0 = { origin.x - dims.x, origin.y - dims.y, origin.z + dims.z }; // (-, -, +)
		triangle11.V1 = { origin.x + dims.x, origin.y - dims.y, origin.z + dims.z }; // (+, -, +)
		triangle11.V2 = { origin.x + dims.x, origin.y - dims.y, origin.z - dims.z }; // (+, -, -)

		glm::vec3 normal0 = glm::vec3(0, 0, -1);
		glm::vec3 normal1 = glm::vec3(1, 0, 0);
		glm::vec3 normal2 = glm::vec3(0, 0, 1);
		glm::vec3 normal3 = glm::vec3(-1, 0, 0);
		glm::vec3 normal4 = glm::vec3(0, 1, 0);
		glm::vec3 normal5 = glm::vec3(0, -1, 0);

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

		Normals[0] = normal0;
		Normals[1] = normal1;
		Normals[2] = normal2;
		Normals[3] = normal3;
		Normals[4] = normal4;
		Normals[5] = normal5;
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
};