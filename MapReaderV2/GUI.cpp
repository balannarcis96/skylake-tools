#include "App.h"

int GuiManager::Initialize(int width, int height)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGuiContext = ImGui::CreateContext();
	if (!ImGuiContext.Get()) {
		return 1;
	}

	EnableKeyboard();
	StyleDark();

	if (!ImGui_ImplWin32_Init(GApp.GetWindowHandle())) {
		return 2;
	}

	if (!ImGui_ImplDX11_Init(GD3DDriver.device.Get(), GD3DDriver.context.Get())) {
		return 3;
	}

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	RegisterUI(&Toolbar);

	return 0;
}

void GuiManager::Update(float dt)
{
}
//bool show_demo_window = true;
void GuiManager::Render(float dt)
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow(&show_demo_window);

	auto Stage = GApp.GetCurrentStage();

	switch (Stage)
	{
	case AppStage::Initialization:
	case AppStage::StartupConfig:
		GConfigTool.RenderUI();
		break;
	case AppStage::Run:
		for (size_t i = 0; i < (int)UIElementType::MAX; i++)
		{
			if (UIEmlements[i].Get()) {
				UIEmlements[i]->Render();
			}
		}
		break;
	default:
		break;
	}

	// End the Dear ImGui frame
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void GuiManager::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

//Main gui elements
void Toolbar::RenderUI()
{
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("Config"))
		{
			GGUIManager.TryShowElement<UIElementType::ConfigTool>();
		}
		if (ImGui::MenuItem("Exit"))
		{
			GApp.CloseApp();
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Tools")) {
		if (ImGui::MenuItem("Scene"))
		{
			GGUIManager.TryShowElement<UIElementType::Scene>();
		}

		if (ImGui::MenuItem("Packages"))
		{
			GGUIManager.TryShowElement<UIElementType::PackagesTool>();
		}

		if (ImGui::MenuItem("Navmesh"))
		{
			GGUIManager.TryShowElement<UIElementType::NavmeshTool>();
		}

		if (ImGui::MenuItem("Settings"))
		{
			GGUIManager.TryShowElement<UIElementType::Settings>();
		}

		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

//Globals
GuiManager		GGUIManager;
ULONG			GuiManager::IdPool;
