#pragma once

#include "Window/Window.h"
#include "Renderer/Renderer.h"

class Application
{
public:
	Application();
	~Application();

	void Run();

private:
	Window* m_Window = nullptr;
	Renderer* m_Renderer = nullptr;
};