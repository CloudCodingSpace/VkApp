#include "Application/Application.h"

int main(int argc, const char** argv)
{
	std::unique_ptr<Application> app = std::make_unique<Application>();
	app->Run();

	return 0;
}