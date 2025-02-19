#include "Application.h"

Application::Application()
{
	m_Window = std::make_shared<Window>(800, 600, "VkApp");
	m_Renderer = std::make_shared<Renderer>(m_Window);
}

void Application::Run()
{
	m_Window->Show();
	m_Renderer->SetClearColor(0.1f, 0.1f, 0.1f);
	while (m_Window->IsOpened())
	{
		m_Renderer->Render();

		m_Renderer->Update();
		m_Window->Update();
	}
}
