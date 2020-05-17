#pragma once

struct PackagesTool : UIElement<UIElementType::PackagesTool>{
	bool Initialize();
	void RenderUI() override;
	bool LoadPackage(const char* path);
	void Reset() {
		GScene.UnloadAllPackages();
		SelectedPackage = nullptr;
		AllPackagesPaths.clear();
	}
	void UnloadAllPackages();

	TRef<UPackage> GetSelectedPackage() const noexcept {
		return SelectedPackage;
	}

private:
	std::vector<std::string>	AllPackagesPaths;
	TRef<UPackage>				SelectedPackage = nullptr;
};