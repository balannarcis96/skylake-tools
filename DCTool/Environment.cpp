#include "App.h"

extern SettingsTool			GSettingsTool;
extern DCTool				GDCTool;

bool InitializeEnv()
{
	if (!GSettingsTool.Initialize()) {
		return false;
	}

	if (!GDCTool.Initialize()) {
		return false;
	}

	return true;
}
