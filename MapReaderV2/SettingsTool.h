#pragma once


struct SettingsTool : UIElement<UIElementType::Settings> {
	bool Initialize();
	void RenderUI() override;

	void SetActiveTab(int tab) noexcept {
		ActiveTab = tab;
	}

	void Reset() {
		
	}

private:

	void RenderTabs() {

		ImGui::PushStyleColor(0, TAB_COLOR(ActiveTab, 0));
		if (ImGui::Button("Main")) {
			ActiveTab = 0;
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(0, TAB_COLOR(ActiveTab, 1));
		if (ImGui::Button("Second")) {
			ActiveTab = 1;
		}
		ImGui::PopStyleColor();

		ImGui::Separator();
	}

	int	ActiveTab = 0;
};