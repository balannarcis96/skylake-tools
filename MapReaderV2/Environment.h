#pragma once

static struct CookedPcEntry { const char* name; int32 bValid; } cookedPcFiles[] = {
   {"Editor.u",								FALSE},
   {"Engine.u",								FALSE},
   //{"GameFramework.u",						FALSE},
   //{"GFxIME.upk",							FALSE},
   //{"GFxUI.u",								FALSE},
   //{"GFxUIEditor.u",						FALSE},
   //{"GlobalPersistentCookerData.upk",		FALSE},
   //{"GlobalShaderCache-PC-D3D-SM3.bin",		FALSE},
   //{"GodHand.u",							FALSE},
   //{"GuidCache.upk",						FALSE},
   //{"IpDrv.u",								FALSE},
   //{"MyDecalTest.gpk",						FALSE},
   //{"Quest.gpk",							FALSE},
   //{"RefShaderCache-PC-D3D-SM3.gpk",		FALSE},
   //{"S1Game.u",								FALSE},
   //{"S1Game_LOC_int.upk",					FALSE},
   //{"SoundTest.gpk",						FALSE},
   //{"SpeechRecognition.gpk",				FALSE},
   //{"Startup_int.upk",						FALSE},
   //{"Startup_USA.upk",						FALSE},
   //{"UnrealEd.u",							FALSE},
   //{"UnrealScriptTest.u",					FALSE}
};
static CookedPcEntry cookedPcDirectories[] = {
   {"Art_Data",								FALSE},
   //{"BasePackages",							FALSE},
   //{"Sound_Data",							FALSE}
};
#define COOKEDPC_ARRAY_COUNT(a) (sizeof(a) / sizeof(CookedPcEntry))

namespace CookedPC {
	static const char* SupportedPackagesExtensions[4] = {
		"gpk",
		"u",
		"upk",
		"gmp"
	};

	inline void ResetEntries() {
		const uint32 dirCount = COOKEDPC_ARRAY_COUNT(cookedPcDirectories);
		const uint32 fileCount = COOKEDPC_ARRAY_COUNT(cookedPcFiles);

		for (uint32 i = 0; i < dirCount; i++)
		{
			cookedPcDirectories[i].bValid = FALSE;
		}

		for (uint32 i = 0; i < fileCount; i++)
		{
			cookedPcFiles[i].bValid = FALSE;
		}

	}
	inline std::vector<std::string> ValidateEntries(const char* root_path) {
		const uint32 dirCount = COOKEDPC_ARRAY_COUNT(cookedPcDirectories);
		const uint32 fileCount = COOKEDPC_ARRAY_COUNT(cookedPcFiles);
		std::string path;
		std::string err;
		bool valid = true;

		std::vector<std::string> errorsMsgs;

		for (uint32 i = 0; i < dirCount; i++)
		{
			path = root_path;
			path += cookedPcDirectories[i].name;
			cookedPcDirectories[i].bValid = DirectoryExists(path.c_str());
			if (!cookedPcDirectories[i].bValid)
			{
				err = "Could not find directory[";
				err += cookedPcDirectories[i].name;
				err += "].";
				errorsMsgs.push_back(std::string((char*)err.c_str(), err.length()));
				valid = false;
			}
		}

		for (uint32 i = 0; i < fileCount; i++)
		{
			path = root_path;
			path += cookedPcFiles[i].name;
			cookedPcFiles[i].bValid = FileExists(path.c_str());
			if (!cookedPcFiles[i].bValid)
			{
				err = "Could not find package[";
				err += cookedPcFiles[i].name;
				err += "].";
				errorsMsgs.push_back(std::string((char*)err.c_str(), err.length()));
				valid = false;
			}
		}

		return std::move(errorsMsgs);
	}

	inline int _isSupportedExt(const char* ext) {
		if (!ext || strlen(ext) < 1) {

		}

		for (uint8 i = 0; i < 4; i++) {
			if (strcmp(ext, SupportedPackagesExtensions[i]) == 0) {
				return i;
			}
		}

		return -1;
	}
	inline const char* _findExtensionInPlace(const char* fileName) {
		const char* result = fileName;

		while (*result != '\0' && *result != '.') {
			result++;
		}

		if (*result == '.')
			result++;

		return result;
	}
	inline int _ScanForPackagesRecursive(std::vector<std::string>& outPackages, const char* path) {
		std::string subPath = path;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAA ffd;
		int32 result = 0;

		hFind = FindFirstFileA(path, &ffd);
		if (hFind == INVALID_HANDLE_VALUE) {
			return 1;
		}
		do
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (strlen(ffd.cFileName) == 0 || ffd.cFileName[0] == '.')
					continue;
				std::string ttPath = subPath;

				ttPath.erase(ttPath.begin() + ttPath.length() - 1, ttPath.end());
				ttPath += ffd.cFileName;
				ttPath += '\\';
				ttPath += '*';

				_ScanForPackagesRecursive(outPackages, ttPath.c_str());
			}
			else {
				const char* ext = _findExtensionInPlace(ffd.cFileName);
				int32 extValid = _isSupportedExt(ext);
				if (extValid != -1) {
					std::string finPath = subPath;
					finPath.erase(finPath.begin(), finPath.begin() + strlen(packagesRootPath));
					finPath.erase(finPath.begin() + finPath.length() - 1, finPath.end());
					finPath += ffd.cFileName;

					outPackages.push_back(std::move(finPath));
				}
			}
		} while (FindNextFileA(hFind, &ffd) != 0);

		FindClose(hFind);
		return 0;
	}

	inline int ScanForPackages(const char* root_path, std::vector<std::string>& outPackages) {
		WIN32_FIND_DATAA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		int32 result = 0;
		std::string subPath = root_path;
		subPath += "*";

		hFind = FindFirstFileA(subPath.c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE) {
			return 1;
		}

		do
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (strlen(ffd.cFileName) == 0 || ffd.cFileName[0] == '.')
					continue;

				subPath = packagesRootPath;
				subPath += ffd.cFileName;
				subPath += '\\';
				subPath += '*';

				_ScanForPackagesRecursive(outPackages, subPath.c_str());
			}
			else {
				char* contex = NULL;
				const char* ext = _findExtensionInPlace(ffd.cFileName);
				int32 extValid = _isSupportedExt(ext);
				if (extValid != -1) {
					outPackages.push_back(std::string(ffd.cFileName, strlen(ffd.cFileName)));
				}
			}
		} while (FindNextFileA(hFind, &ffd) != 0);

		FindClose(hFind);

		std::sort(outPackages.begin(), outPackages.end(), [](std::string& first, std::string& next)->bool {
			return strcmp(first.c_str(), next.c_str()) < 0 /*&& first.length() < next.length()*/;
			});

		return 0;
	}
}

#include "PackagesTool.h"
#include "NavmeshTool.h"
#include "ConfigTool.h"
#include "SettingsTool.h"

bool InitializeEnv();