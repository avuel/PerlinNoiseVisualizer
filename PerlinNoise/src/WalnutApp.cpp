#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

#include "Renderer.hpp"
#include "Camera.hpp"

#include <memory>
#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f) 
	{
	
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Camera.OnUpdate(ts));
	}
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);

		ImGui::Checkbox("Parallel Rendering", &m_Renderer.GetSettings().Parallel);
		ImGui::Checkbox("Render Noise Map", &m_Renderer.GetSettings().Noise);
		ImGui::Checkbox("Octree Optimization (Recommended)", &m_Renderer.GetSettings().OcTree);
		if (ImGui::Checkbox("Color Height Map", &m_Scene.GetNoiseSettings()->Color));

		float speed = m_Camera.GetSpeed();
		ImGui::PushItemWidth(120);
		if (ImGui::InputFloat("Camera Speed", &speed, 0.0f, 0.0f, "%.3f"))
		{
			speed = std::clamp(speed, std::numeric_limits<float>::min(), 250.0f);
			m_Camera.SetSpeed(speed);
		}
		ImGui::PopItemWidth();
		
		ImGui::PushItemWidth(150);
		int oldHeight = m_Scene.GetNoiseHeight();
		if (ImGui::InputInt("Height Scale", &m_Scene.GetNoiseSettings()->Height, 1, 1))
		{
			int newHeight = m_Scene.GetNoiseSettings()->Height;
			if (oldHeight < newHeight)
				newHeight = oldHeight * 2;
			else
				newHeight = oldHeight / 2;

			// Clamp height values and update cell size if necessary
			newHeight = std::clamp(newHeight, 1, 256);
			m_Scene.SetNoiseHeight(newHeight);
		}
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(120);

		if (ImGui::InputFloat("Water Height", &m_Scene.GetNoiseSettings()->Water, 0.0f, 0.0f, "%.3f"))
		{
			float newWater = std::clamp(m_Scene.GetNoiseSettings()->Water, 0.0f, 1.0f);
			m_Scene.SetNoiseWater(newWater);
		}
		if (ImGui::InputFloat("Sand Height", &m_Scene.GetNoiseSettings()->Sand, 0.0f, 0.0f, "%.3f"))
		{
			float newSand = std::clamp(m_Scene.GetNoiseSettings()->Sand, 0.0f, 1.0f);
			m_Scene.SetNoiseSand(newSand);
		}

		if (ImGui::InputFloat("Stone Height", &m_Scene.GetNoiseSettings()->Stone, 0.0f, 0.0f, "%.3f"))
		{
			float newStone = std::clamp(m_Scene.GetNoiseSettings()->Stone, 0.0f, 1.0f);
			m_Scene.SetNoiseStone(newStone);
		}

		if (ImGui::InputFloat("Snow Height", &m_Scene.GetNoiseSettings()->Snow, 0.0f, 0.0f, "%.3f"))
		{
			float newSnow = std::clamp(m_Scene.GetNoiseSettings()->Snow, 0.0f, 1.0f);
			m_Scene.SetNoiseSnow(newSnow);
		}
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(120);
		if (ImGui::InputFloat("Water Height", &m_Renderer.GetSettings().Water, 0.0f, 0.0f, "%.3f"))
		{
			speed = std::clamp(speed, std::numeric_limits<float>::min(), 100.0f);
			m_Camera.SetSpeed(speed);
		}
		if (ImGui::InputFloat("Sand Height", &m_Renderer.GetSettings().Sand, 0.0f, 0.0f, "%.3f"))
		{
			speed = std::clamp(speed, std::numeric_limits<float>::min(), 100.0f);
			m_Camera.SetSpeed(speed);
		}

		if (ImGui::InputFloat("Stone Height", &m_Renderer.GetSettings().Stone, 0.0f, 0.0f, "%.3f"))
		{
			speed = std::clamp(speed, std::numeric_limits<float>::min(), 100.0f);
			m_Camera.SetSpeed(speed);
		}

		if (ImGui::InputFloat("Snow Height", &m_Renderer.GetSettings().Snow, 0.0f, 0.0f, "%.3f"))
		{
			speed = std::clamp(speed, std::numeric_limits<float>::min(), 100.0f);
			m_Camera.SetSpeed(speed);
		}
		ImGui::PopItemWidth();

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar);

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
