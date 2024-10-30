#include "Application.h"

Application::Application()
{
	m_Window = new Window(800, 600, "An Appliation using Vulkan");
}

Application::~Application()
{
	delete m_Renderer;
	delete m_Window;
}

void Application::Run()
{
	m_Window->Show();
	while (m_Window->IsOpened())
	{
		m_Renderer->Render();

		m_Renderer->Update();
		m_Window->Update();
	}
}
