#include "WinApi.h"

INT32 Timer::Init()
{
	if (!QueryPerformanceFrequency(&I))
	{
		return 1;
	}

	FrequencySeconds = (float)(I.QuadPart);
	QueryPerformanceCounter(&I);
	Start = I.QuadPart;
	TotalTime = 0.0;
	Elapsed = 0.0;

	return 0;
}

void GetDesktopResolution(int& horizontal, int& vertical)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
}
