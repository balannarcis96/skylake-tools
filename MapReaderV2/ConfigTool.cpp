#include "App.h"

constexpr char DefaultPath[] = "D:\\Tera_Packages\\CookedPC\\Art_Data\\Maps\\Velika\\";
constexpr char DefaultResourcePath[] = ".\\resources";

bool ConfigTool::Initialize()
{
	auto Viewport = GD3DDriver.GetViewport();
	Width = Viewport.Width / 2;
	Height = Viewport.Height / 2;

	if (!GGUIManager.RegisterUI<UIElementType::ConfigTool>(this)) {
		return 1;
	}

	strcpy_s(CookedPCFilePath, DefaultPath);

	return true;
}

void ConfigTool::RenderUI()
{
	if (GApp.GetCurrentStage() == AppStage::Initialization)
	{
		/*auto Viewport = GD3DDriver.GetViewport();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

		ImGui::SetNextWindowPos(ImVec2(Viewport.Width / 4, Viewport.Height / 4));
		ImGui::SetNextWindowSize(ImVec2(Width, Height));
		ImGui::Begin("Info", &bDisplay, window_flags);

		ImGui::Text("Initializing...");

		ImGui::End();*/
	}
	else if (GApp.GetCurrentStage() == AppStage::StartupConfig)
	{
		auto Viewport = GD3DDriver.GetViewport();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove /*| ImGuiWindowFlags_NoScrollbar*/ | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

		ImGui::SetNextWindowPos(ImVec2(Viewport.Width / 4, Viewport.Height / 4));
		ImGui::SetNextWindowSize(ImVec2(Width, Height));
		ImGui::Begin("Config", &bDisplay, window_flags);

		ImGui::TextColored(INFO_COLOR, "Resource Path:%s", DefaultResourcePath);
		ImGui::TextColored(INFO_COLOR, "Packages folder Path( must end with \\)");
		ImGui::SetNextItemWidth(Width - 15.0f);
		ImGui::InputText("", CookedPCFilePath, 512);

		ImGui::Checkbox("Force CookedPC Folder check", &IsForcedCookedPCFolder);

		if (ImGui::Button("Finalize")) {

			bool Valid = false;

			Valid = DirectoryExists(CookedPCFilePath);

			if (Valid && IsForcedCookedPCFolder) {
				Errors = CookedPC::ValidateEntries(CookedPCFilePath);
				Valid = Errors.size() == 0;
			}

			if (Valid) {
				Close(); //close config window
				GApp.SetStage(AppStage::Run);
			}

			if (!Valid) {
				CookedPC::ResetEntries();
				if (IsForcedCookedPCFolder) {
					Message("Invalid CookedPC Folder Path [%s]", CookedPCFilePath);
				}
				else {
					Message("Invalid Path [%s]", CookedPCFilePath);
				}
			}
		}

		if (Errors.size()) {
			ImGui::Separator();
			for (auto& Err : Errors)
			{
				ImGui::TextColored(ERROR_COLOR, Err.c_str());
			}

			if (ImGui::Button("Clear Errors")) {
				ClearErrors();
			}
		}
		ImGui::Separator();
		ImGui::End();
	}
	else if (GApp.GetCurrentStage() == AppStage::Run) {
		ImGui::Begin("Config", &bDisplay);
		ImGui::Text("Config:");

		ImGui::Separator();
		ImGui::TextColored(INFO_COLOR, "Packages folder Path:");
		ImGui::Text(CookedPCFilePath);
		ImGui::TextColored(INFO_COLOR, "Is CookedPc Folder:");
		ImGui::Text(IsForcedCookedPCFolder ? "Yes" : "No");
		ImGui::Separator();

		//RenderCloseButton();
		ImGui::End();
	}
}

void ConfigTool::Reset()
{
	IsForcedCookedPCFolder = false;
	strcpy_s(CookedPCFilePath, DefaultPath);
}

std::string ConfigTool::BuildShaderPath(const char* ShaderName)
{
	std::string Path = DefaultResourcePath;

	Path += "\\shaders\\";
	Path += ShaderName;

	return std::move(Path);
}

ConfigTool GConfigTool;