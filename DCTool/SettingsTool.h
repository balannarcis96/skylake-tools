#pragma once

struct SettingsTool : UIElement<UIElementType::Settings> {
	bool Initialize();
	void RenderUI() override;
	void Reset() {
		
	}

private:
};