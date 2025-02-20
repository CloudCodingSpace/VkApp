#pragma once

#include "Window/Window.h"
#include "Renderer/Renderer.h"

class Application
{
public:
	Application();

	void Run();
private:
	Window m_Window;
	Renderer m_Renderer;
};