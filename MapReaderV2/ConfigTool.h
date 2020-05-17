#pragma once


struct ConfigTool : UIElement<UIElementType::ConfigTool> {
	bool Initialize();

	void RenderUI() override;

	void Reset();
	void ClearErrors() {
		Errors.clear();
	}

	std::string BuildPackagePath(const char* PackageName) {
		std::string Path = CookedPCFilePath;

		Path += PackageName;

		return std::move(Path);
	}
	std::string	BuildShaderPath(const char* ShaderName);

	//Config
	bool						IsForcedCookedPCFolder = false;
	char						CookedPCFilePath[512];

	//Erros
	std::vector<std::string>	Errors;
};

extern ConfigTool GConfigTool;
