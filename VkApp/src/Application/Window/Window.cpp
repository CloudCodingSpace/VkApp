#include "Window.h"

#include <stdexcept>

Window::Window(int width, int height, std::string title) : m_Width{width}, m_Height{height}
{
	if (!glfwInit())
		throw std::runtime_error("Failed to init GLFW!");

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_Handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!m_Handle)
		throw std::runtime_error("Failed to create the window!");
}

Window::~Window()
{
	glfwDestroyWindow(m_Handle);
	glfwTerminate();
}

void Window::Show()
{
	glfwShowWindow(m_Handle);
}

void Window::Update(std::function<void(void)> callback)
{
	if (callback)
		callback();

	glfwPollEvents();
}
