#pragma once

#include "Window/Window.h"
#include "Renderer/Renderer.h"

class Application
{
public:
	Application();

	void Run();
private:
	std::shared_ptr<Window> m_Window;
	std::shared_ptr<Renderer> m_Renderer;
};