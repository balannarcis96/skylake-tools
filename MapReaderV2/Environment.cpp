#include "App.h"

extern NavmeshTool			GNavmeshTool;
extern PackagesTool			GPackageTool;
extern SettingsTool			GSettingsTool;
bool InitializeEnv()
{
	if (!GPackageTool.Initialize()) {
		return false;
	}

	if (!GNavmeshTool.Initialize()) {
		return false;
	}

	if (!GConfigTool.Initialize()) {
		return false;
	}

	if (!GSettingsTool.Initialize()) {
		return false;
	}

	return true;
}
