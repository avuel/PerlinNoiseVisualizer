#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

#include "Renderer.hpp"
#include "Camera.hpp"

#include <memory>
#include <charconv>
#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f) 
	{
		Material &ambientSky = m_Scene.Materials.emplace_back();
		m_Scene.ObjectNames.push_back("Sky");
		ambientSky.Albedo = { 0.6f, 0.7f, 0.9f };

		Material &graySphere = m_Scene.Materials.emplace_back();
		m_Scene.ObjectNames.push_back("Gray Sphere");
		graySphere.Albedo = { 0.125f, 0.125f, 0.125f };
		graySphere.Reflectiveness = 0.4;
		graySphere.Shinyness = 32;

		Material &ground = m_Scene.Materials.emplace_back();
		m_Scene.ObjectNames.push_back("Ground (large sphere)");
		ground.Albedo = { 0.1f, 0.1f, 0.1f };
		ground.Reflectiveness = 0.2f;
		ground.Shinyness = 512;

		Material &redCube = m_Scene.Materials.emplace_back();
		m_Scene.ObjectNames.push_back("Red AABB");
		redCube.Albedo = { 1.0f, 0.0f, 0.0f };
		redCube.Reflectiveness = 0.0f;
		redCube.Shinyness = 8;

		{
			Sphere sphere;
			sphere.Origin = { 0.0f, 0.2f, 0.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 1;
			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Origin = { 0.0f, -101.0f, 0.0f };
			sphere.Radius = 100.0f;
			sphere.MaterialIndex = 2;
			m_Scene.Spheres.push_back(sphere);
		}
		
		{
			Cube cube;
			float size = 0.5f; // Half the side length
			glm::vec3 origin = { -2.0f, 0.0f, 2.0f };
			cube.Origin = origin;

			// Back Face
			Triangle triangle0;
			triangle0.V0 = { origin.x - size, origin.y - size, origin.z - size }; // (-, -, -)
			triangle0.V1 = { origin.x + size, origin.y - size, origin.z - size }; // (+, -, -)
			triangle0.V2 = { origin.x - size, origin.y + size, origin.z - size }; // (-, +, -)
			Triangle triangle1;
			triangle1.V0 = { origin.x + size, origin.y - size, origin.z - size }; // (+, -, -)
			triangle1.V1 = { origin.x + size, origin.y + size, origin.z - size }; // (+, +, -)
			triangle1.V2 = { origin.x - size, origin.y + size, origin.z - size }; // (-, +, -)

			// Right Face
			Triangle triangle2;
			triangle2.V0 = { origin.x + size, origin.y - size, origin.z - size }; // (+, -, -)
			triangle2.V1 = { origin.x + size, origin.y - size, origin.z + size }; // (+, -, +)
			triangle2.V2 = { origin.x + size, origin.y + size, origin.z - size }; // (+, +, -)
			Triangle triangle3;
			triangle3.V0 = { origin.x + size, origin.y - size, origin.z + size }; // (+, -, +)
			triangle3.V1 = { origin.x + size, origin.y + size, origin.z + size }; // (+, +, +)
			triangle3.V2 = { origin.x + size, origin.y + size, origin.z - size }; // (+, +, -)

			// Front Face
			Triangle triangle4;
			triangle4.V0 = { origin.x + size, origin.y - size, origin.z + size }; // (+, -, +)
			triangle4.V1 = { origin.x - size, origin.y - size, origin.z + size }; // (-, -, +)
			triangle4.V2 = { origin.x + size, origin.y + size, origin.z + size }; // (+, +, +)
			Triangle triangle5;
			triangle5.V0 = { origin.x - size, origin.y - size, origin.z + size }; // (-, -, +)
			triangle5.V1 = { origin.x - size, origin.y + size, origin.z + size }; // (-, +, +)
			triangle5.V2 = { origin.x + size, origin.y + size, origin.z + size }; // (+, +, +)

			// Left Face
			Triangle triangle6;
			triangle6.V0 = { origin.x - size, origin.y - size, origin.z + size }; // (-, -, +)
			triangle6.V1 = { origin.x - size, origin.y - size, origin.z - size }; // (-, -, -)
			triangle6.V2 = { origin.x - size, origin.y + size, origin.z + size }; // (-, +, +)
			Triangle triangle7;
			triangle7.V0 = { origin.x - size, origin.y - size, origin.z - size }; // (-, -, -)
			triangle7.V1 = { origin.x - size, origin.y + size, origin.z - size }; // (-, +, -)
			triangle7.V2 = { origin.x - size, origin.y + size, origin.z + size }; // (-, +, +)

			// Top Face
			Triangle triangle8;
			triangle8.V0 = { origin.x - size, origin.y + size, origin.z - size }; // (-, +, -)
			triangle8.V1 = { origin.x + size, origin.y + size, origin.z - size }; // (+, +, -)
			triangle8.V2 = { origin.x - size, origin.y + size, origin.z + size }; // (-, +, +)
			Triangle triangle9;
			triangle9.V0 = { origin.x + size, origin.y + size, origin.z - size }; // (+, +, -)
			triangle9.V1 = { origin.x + size, origin.y + size, origin.z + size }; // (+, +, +)
			triangle9.V2 = { origin.x - size, origin.y + size, origin.z + size }; // (-, +, +)

			// Bottom Face
			Triangle triangle10;
			triangle10.V0 = { origin.x - size, origin.y - size, origin.z - size }; // (-, -, -)
			triangle10.V1 = { origin.x - size, origin.y - size, origin.z + size }; // (-, -, +)
			triangle10.V2 = { origin.x + size, origin.y - size, origin.z - size }; // (+, -, -)
			Triangle triangle11;
			triangle11.V0 = { origin.x - size, origin.y - size, origin.z + size }; // (-, -, +)
			triangle11.V1 = { origin.x + size, origin.y - size, origin.z + size }; // (+, -, +)
			triangle11.V2 = { origin.x + size, origin.y - size, origin.z - size }; // (+, -, -)

			glm::vec3 normal0 = glm::vec3(0, 0, -1);
			glm::vec3 normal1 = glm::vec3(0, 0, -1);
			glm::vec3 normal2 = glm::vec3(1, 0, 0);
			glm::vec3 normal3 = glm::vec3(1, 0, 0);
			glm::vec3 normal4 = glm::vec3(0, 0, 1);
			glm::vec3 normal5 = glm::vec3(0, 0, 1);
			glm::vec3 normal6 = glm::vec3(-1, 0, 0);
			glm::vec3 normal7 = glm::vec3(-1, 0, 0);
			glm::vec3 normal8 = glm::vec3(0, 1, 0);
			glm::vec3 normal9 = glm::vec3(0, 1, 0);
			glm::vec3 normal10 = glm::vec3(0, -1, 0);
			glm::vec3 normal11 = glm::vec3(0, -1, 0);

			cube.Triangles[0] = triangle0;
			cube.Triangles[1] = triangle1;
			cube.Triangles[2] = triangle2;
			cube.Triangles[3] = triangle3;
			cube.Triangles[4] = triangle4;
			cube.Triangles[5] = triangle5;
			cube.Triangles[6] = triangle6;
			cube.Triangles[7] = triangle7;
			cube.Triangles[8] = triangle8;
			cube.Triangles[9] = triangle9;
			cube.Triangles[10] = triangle10;
			cube.Triangles[11] = triangle11;
			
			cube.Normals[0] = normal0;
			cube.Normals[1] = normal1;
			cube.Normals[2] = normal2;
			cube.Normals[3] = normal3;
			cube.Normals[4] = normal4;
			cube.Normals[5] = normal5;
			cube.Normals[6] = normal6;
			cube.Normals[7] = normal7;
			cube.Normals[8] = normal8;
			cube.Normals[9] = normal9;
			cube.Normals[10] = normal10;
			cube.Normals[11] = normal11;

			cube.MaterialIndex = 3;

			m_Scene.Cubes.push_back(cube);
		}
		
		{
			glm::vec3 lightDirection = glm::vec3(-1.0f, -1.0f, 0.0f);
			glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
			m_Scene.Lights.emplace_back(new DirectionalLight(lightDirection, lightColor));
		}

		{
			glm::vec3 lightOrigin = { 1.0f, 1.1f, 0.4f };
			glm::vec3 lightColor = { 1.0f, 1.0f, 0.0f };
			m_Scene.Lights.emplace_back(new PointLight(lightOrigin, lightColor));
		}
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();

	}
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);

		ImGui::Checkbox("Parallel Rendering", &m_Renderer.GetSettings().Parallel);
		
		ImGui::End();

		ImGui::Begin("Scene");

		ImGui::Text("Sky");
		ImGui::ColorEdit3("Albedo", glm::value_ptr(m_Scene.Materials[0].Albedo), 0.1f);
		ImGui::Separator();

		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere &sphere = m_Scene.Spheres[i];
			ImGui::Text(m_Scene.ObjectNames[sphere.MaterialIndex]);
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Origin), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.01f, 0.0f);

			Material &material = m_Scene.Materials[sphere.MaterialIndex];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo), 0.1f);
			ImGui::DragFloat("Reflective", &material.Reflectiveness, 0.0f, 0.0f, 1.0f);
			const int oldshinyness = material.Shinyness;
			if (ImGui::InputInt("Shinyness", &material.Shinyness))
			{
				if (material.Shinyness < oldshinyness)
					material.Shinyness = oldshinyness / 2;
				else
					material.Shinyness = oldshinyness * 2;

				material.Shinyness = std::clamp(material.Shinyness, 1, 4096);
			}

			ImGui::Separator();

			ImGui::PopID();
		}

		for (size_t i = 0; i < m_Scene.Cubes.size(); i++)
		{
			ImGui::PushID(i + m_Scene.Spheres.size());

			Cube &cube = m_Scene.Cubes[i];
			glm::vec3 oldOrigin = cube.Origin;
			ImGui::Text(m_Scene.ObjectNames[cube.MaterialIndex]);
			if (ImGui::DragFloat3("Position", glm::value_ptr(cube.Origin), 0.1f))
			{
				cube.Translate(cube.Origin - oldOrigin);
			}

			Material &material = m_Scene.Materials[cube.MaterialIndex];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo), 0.1f);
			ImGui::DragFloat("Reflective", &material.Reflectiveness, 0.0f, 0.0f, 1.0f);
			const int oldshinyness = material.Shinyness;
			if (ImGui::InputInt("Shinyness", &material.Shinyness))
			{
				if (material.Shinyness < oldshinyness)
					material.Shinyness = oldshinyness / 2;
				else
					material.Shinyness = oldshinyness * 2;

				material.Shinyness = std::clamp(material.Shinyness, 1, 4096);
			}

			ImGui::Separator();

			ImGui::PopID();
		}

		// Interactive Modifiers for Scene lights
		int lightcount = 0;
		for (auto &light : m_Scene.Lights)
		{
			ImGui::PushID(lightcount);

			
			ImGui::Text("Light");
			ImGui::SameLine();
			ImGui::Text(std::to_string(++lightcount).c_str());
			ImGui::DragFloat3(light->GetType(), glm::value_ptr(*light->GetData()), 0.1f);

			ImGui::Separator();

			ImGui::PopID();
		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if (image)
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }, 
				ImVec2(0,1), ImVec2(1,0));

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
};
 

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracer";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}