#include "App.h"

bool PackagesTool::Initialize()
{
	if (!GGUIManager.RegisterUI<UIElementType::PackagesTool>(this)) {
		return 1;
	}

	return true;
}

void PackagesTool::RenderUI()
{
	ImGui::Begin("Manage Packages", &bDisplay);

	if (ImGui::BeginTabBar("Tabs"))
	{
		if (ImGui::BeginTabItem("Available Packages"))
		{
			if (ImGui::Button("Scan for packages")) {
				Reset();

				CookedPC::ScanForPackages(GConfigTool.CookedPCFilePath, AllPackagesPaths);

				if (!AllPackagesPaths.size()) {
					Message("No packages found!");
				}
			}

			if (AllPackagesPaths.size()) {
				ImGui::Separator();
				ImGui::TextColored(INFO_COLOR, "All Packages(%lld):", AllPackagesPaths.size());

				bool selected = false;

				for (auto& Pkg : AllPackagesPaths)
				{
					selected = ImGui::Selectable(Pkg.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);
					if (selected && ImGui::IsMouseDoubleClicked(0))
					{
						if (GScene.ContainsPackage(Pkg.c_str())) {
							Message("Package already loaded");
						}
						else if (!LoadPackage(Pkg.c_str())) {
							Message("Failed to load package [%s]", Pkg.c_str());
						}
					}
				}
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Loaded Packages"))
		{
			if (ImGui::Button("Unload all pacakges")) {
				UnloadAllPackages();
			}

			ImGui::Separator();
			const auto& LoadedPackage = GScene.GetLoadedPackages();
			if (LoadedPackage.size()) {
				bool selected = false;
				ImGui::TextColored(INFO_COLOR, "Double click to select a package");

				for (auto* Pkg : LoadedPackage)
				{
					selected = ImGui::Selectable(Pkg->PackageName.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick/*, { ImGui::GetWindowWidth() - 100, 20 }*/);
					/*ImGui::SameLine(ImGui::GetWindowWidth() - 85);
					ImGui::Checkbox("Visible", &Pkg->Visible);*/
					if (selected && ImGui::IsMouseDoubleClicked(0))
					{
						SelectedPackage = Pkg;

						GScene.OnPackageSelected(Pkg);
					}
				}
			}
			else {
				ImGui::TextColored(INFO_COLOR, "No packages loaded");
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Package details"))
		{
			if (SelectedPackage.Get()) {
				ImGui::TextColored(INFO_COLOR, "Package: %s", SelectedPackage->PackageName.c_str());

				ImGui::Separator();
				ImGui::Checkbox("Visible", &SelectedPackage->Visible);

				if (ImGui::Button("Release Packet")) {
					SelectedPackage = nullptr;
					GScene.OnPackageSelected(nullptr);
				}
			}
			else {
				ImGui::TextColored(INFO_COLOR, "No package selected");
			}

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}

bool PackagesTool::LoadPackage(const char* packageName)
{
	auto* Package = ::LoadPackage(GConfigTool.BuildPackagePath(packageName).c_str());
	if (!Package) {
		return false;
	}

	Package->PackageNameWithExtension = packageName;

	GScene.AddPackage(Package);

	return true;
}

void PackagesTool::UnloadAllPackages()
{
	GGUIManager.GetElementAs<NavmeshTool>()->Clear();
	SelectedPackage = nullptr;
	GScene.UnloadAllPackages();
}

//Globals
PackagesTool	 GPackageTool;