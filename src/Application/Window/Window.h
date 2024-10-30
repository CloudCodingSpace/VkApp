#pragma once

#include <GLFW/glfw3.h>

#include <string>
#include <functional>

class Window
{
public:
	Window(int width, int height, std::string title);
	~Window();

	void Show();
	inline bool IsOpened() { return !glfwWindowShouldClose(m_Handle); }
	void Update(std::function<void(void)> callback = 0);

	inline GLFWwindow* GetInternHandle() { return m_Handle; }
	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }

private:
	GLFWwindow* m_Handle = nullptr;
	int m_Width, m_Height;
};