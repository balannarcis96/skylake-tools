#pragma once

#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#include <algorithm>
#include <fvec.h>
#include <stdio.h>
#include <ctype.h>

#include "typedefs.h"

#define isZero(v) (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f)

constexpr char DefaultResourcePath[] = "./resources";

typedef __int64 TimeVal;

inline unsigned int nextPow2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

inline unsigned int ilog2(unsigned int v)
{
	unsigned int r;
	unsigned int shift;
	r = (v > 0xffff) << 4; v >>= r;
	shift = (v > 0xff) << 3; v >>= shift; r |= shift;
	shift = (v > 0xf) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3) << 1; v >>= shift; r |= shift;
	r |= (v >> 1);
	return r;
}

inline float Clampf(float val, float min, float max) {
	if (val < min) {
		return min;
	}

	if (val > max) {
		return max;
	}

	return val;
}
inline float WrapAngle(float x) {
	x = (float)fmodf((double)(x + 180), 360);
	if (x < 0)
		x += 360;
	return x - 180;
}

#include "WinApi.h"

inline TimeVal getPerfTime()
{
	__int64 count;
	QueryPerformanceCounter((LARGE_INTEGER*)&count);
	return count;
}

inline int getPerfTimeUsec(const TimeVal duration)
{
	static __int64 freq = 0;
	if (freq == 0)
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	return (int)(duration * 1000000 / freq);
}

#include "SMath/SMathSSE.h"
#include "SMath/SMath.h"

#include "D3D.h"
#include "GUI.h"
#include "Scene.h"
#include "Environment.h"

enum class AppStage {
	Initialization = 0,
	StartupConfig,
	Run
};

struct App {
	int Initialize(HINSTANCE hInstance, int width, int height, int nShowCmd);

	void Run();
	void Shutdown();

	HWND GetWindowHandle() const noexcept {
		return hWnd;
	}

	void CloseApp() {
		if (MessageBox(hWnd, L"Really quit?", L"DC Tool", MB_OKCANCEL) == IDOK)
		{
			DestroyWindow(GetWindowHandle());
		}
	}

	AppStage GetCurrentStage() const noexcept {
		return Stage;
	}
	void SetStage(AppStage Stage)noexcept;
	const Timer& GetTimer()const noexcept { return Timer; }

	bool HasFocus() const noexcept {
		return GetFocus() == hWnd;
	}
private:
	void RegisterMainShaderPack() noexcept;

	void Update();
	void Render();

	int InitWin32Window(HINSTANCE hInstance, int width, int height, int nCmdShow = 0);

	HWND		hWnd = NULL;
	Timer		Timer;

	AppStage	Stage = AppStage::Run;
};

extern App GApp;
