#include "Window.h"

#include "Logger.h"

Window::Window(int width, int height, std::string title) : m_Width{width}, m_Height{height}
{
	if (!glfwInit())
		FATAL("Failed to init GLFW!");

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_Handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!m_Handle)
		FATAL("Failed to create the window!");

	glfwSetWindowUserPointer(m_Handle, this);
	glfwSetFramebufferSizeCallback(m_Handle, SizeCallback);
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

void Window::SizeCallback(GLFWwindow* window, int w, int h)
{
	Window* win = (Window*) glfwGetWindowUserPointer(window);
	win->m_Width = w;
	win->m_Height = h;
}