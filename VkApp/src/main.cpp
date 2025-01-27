#include <iostream>
#include <stdexcept>

#include "Application/Application.h"

int main(int argc, const char** argv)
{
	try {
		Application* app = new Application();
		app->Run();
		delete app;
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}