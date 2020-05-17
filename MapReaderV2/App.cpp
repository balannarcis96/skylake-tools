#include "App.h"

int App::Initialize(HINSTANCE hInstance, int width, int height, int nShowCmd)
{
	if (InitWin32Window(hInstance, width, height, nShowCmd)) {
		Message("Failed to create the Win32Window");
		return 1;
	}

	if (GD3DDriver.Initialize(width, height)) {
		Message("Failed to init D3DDriver");
		return 2;
	}

	RegisterMainShaderPack();

	if (GScene.Initialize()) {
		Message("Failed to init Scene");
		return 3;
	}

	if (GGUIManager.Initialize(width, height)) {
		Message("Failed to init GUI");
		return 4;
	}

	Timer.Init();

	if (!InitializeEnv()) {
		Message("Failed to init Environment");
		return 5;
	}

	SetStage(AppStage::StartupConfig);

	return 0;
}

void App::Run()
{
	MSG msg{};
	memset(&msg, 0, sizeof(MSG));

	while (msg.message != WM_QUIT) {
		if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		Update();
		Render();

		auto Elapsed = Timer.Elapsed;

		if (0.068f - Elapsed > 0.0f) {
			Sleep((DWORD)((0.068f - Elapsed) * 100)); //run at 144 fps
		}
	}
}

void App::Shutdown()
{
	GGUIManager.Shutdown();
	GScene.Shutdown();
	GD3DDriver.Shutdown();
}

void App::SetStage(AppStage Stage) noexcept
{
	switch (Stage)
	{
	case AppStage::Initialization: {
		CookedPC::ResetEntries();
		GConfigTool.Reset();
	}break;
	case AppStage::StartupConfig:
		break;
	case AppStage::Run:
		break;
	default:
		break;
	}

	this->Stage = Stage;
}

void App::RegisterMainShaderPack() noexcept
{
	std::vector<RawDataWrapper<D3D11_INPUT_ELEMENT_DESC>> params = {
		RawDataWrapper<D3D11_INPUT_ELEMENT_DESC>{D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }},
		RawDataWrapper<D3D11_INPUT_ELEMENT_DESC>{D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }},
		RawDataWrapper<D3D11_INPUT_ELEMENT_DESC>{D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }}
	};
	GD3DDriver.RegisterShaderPack(AppShader::MainShaderPack, "MainShaderPShader.hlsl", "MainShaderVShader.hlsl", "MainShaders", params);

	params = {
		RawDataWrapper<D3D11_INPUT_ELEMENT_DESC>{D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }},
		RawDataWrapper<D3D11_INPUT_ELEMENT_DESC>{D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }},
	};
	GD3DDriver.RegisterShaderPack(AppShader::LineShaderPack, "LinePShader.hlsl", "LineVShader.hlsl", "LineShaders", params);
}

void App::Update()
{
	auto Elapsed = Timer.Elapsed;
	auto TotalTime = Timer.Tick();

	if (Stage == AppStage::Run) {
		GScene.Update(Elapsed);
	}

	GGUIManager.Update(Elapsed);
}

void App::Render()
{
	auto Elapsed = Timer.Elapsed;

	GD3DDriver.D3DBegin(GScene.ClearColor);

	if (Stage == AppStage::Run) {
		GScene.Render(Elapsed);
	}

	GGUIManager.Render(Elapsed);

	GD3DDriver.D3DEnd();
}

#pragma region INIT_WIN32

LRESULT CALLBACK ProcCallback(HWND   hWnd, UINT   msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return 0;
	}

	switch (msg)
	{
	case WM_CLOSE:
		GApp.CloseApp();
		// Else: User canceled. Do nothing.
		return 0;
		//case WM_SYSCOMMAND:
		//	if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
		//		return 0;
		//	break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	}break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

int App::InitWin32Window(HINSTANCE hInstance, int width, int height, int nCmdShow)
{
	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(windowClass));

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = ProcCallback;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	windowClass.lpszClassName = L"TERA_MAP_REANDER_WIN32";
	windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&windowClass)) {
		return 1;
	}

	hWnd = CreateWindowEx(
		0,
		L"TERA_MAP_REANDER_WIN32",
		L"Tera Map Reader v2.0 (by Narcis96)",
		WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		0, 0,
		width,
		height,
		NULL,
		NULL,
		hInstance,
		NULL);
	if (!hWnd) {
		return 2;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return 0;
}

#pragma endregion

//Globals
App GApp;
