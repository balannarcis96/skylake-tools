#include "App.h"

#include <list>

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	int width = 1280, height = 720;
	GetDesktopResolution(width, height);

	int Result = GApp.Initialize(hInstance, 1024, 768, nShowCmd);
	if (Result) {
		Message("Initialization failed");
		return 1;
	}

	GApp.Run();

	GApp.Shutdown();

	return 0;
}
