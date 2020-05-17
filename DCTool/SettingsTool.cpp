#include "App.h"

bool SettingsTool::Initialize()
{
	if (!Base::Initialize()) {
		return false;
	}

	Close();

	return true;
}

void SettingsTool::RenderUI()
{
	ImGui::Begin("Settings", &bDisplay);

	if (ImGui::BeginTabBar("Tabs"))
	{
		if (ImGui::BeginTabItem("Main"))
		{
			ImGui::Text("Main...");
			
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Second"))
		{
			ImGui::Text("Second...");

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

SettingsTool GSettingsTool;