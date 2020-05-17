#include "PackageManager.h"

#include "StaticMesh.h"
#include "Actor.h"
#include "Terrain.h"
#include "TerrainComponent.h"
#include "Level.h"

#include <algorithm>

std::vector<std::string> allPackagesPaths;
std::vector<ActorsExportEntry*> sceneExport;
bool exportOnlyObjectsIfNotOnlyTerrain;
bool partitionSceneObjects;
int objectsPerPartitions;
char exportObjsFileName[512];

const char* SupportedPackagesExtensions[4] = {
	"gpk",
	"u",
	"upk",
	"gmp"
};

void SetPackagesRootPath(const char* path)
{
	if (topIndex || !path) {
		return;
	}

	int32 pathLen = strlen(path) + 1;
	if (!pathLen || pathLen > MAX_PATH) {
		return;
	}

	memcpy_s(packagesRootPath, pathLen, path, pathLen);
}

const char* GetPackagesRootPath()
{
	return packagesRootPath;
}

//#include <zlib.h>
void InitPackageManager()
{
}

std::vector<UPackage*> packages;

UPackage* activePackage = nullptr;
int32 currentStaticMesh = -1;
int32 currentStaticActorMesh = -1;
int32 currentTerrain = -1;

UPackage* GetActivePackage()
{
	return activePackage;
}

UPackage* LoadPackage(const char* packageFilePath) {
	if (!packageFilePath) {
		return nullptr;
	}

	if (activePackage) {
		return nullptr;
	}

	std::string path = packagesRootPath;
	path += packageFilePath;

	UPackage* package = new UPackage(path.c_str());
	if (package->bFailedToLoad) {
		delete package;
		return nullptr;
	}

	//packages.push_back(package);
	package->Init();

	return package;
}

int32 GetLoadedPackagesCount()
{
	return topIndex;
}

int32 _isSupportedExt(const char* ext) {
	if (!ext || strlen(ext) < 1) {

	}

	for (uint8 i = 0; i < 4; i++) {
		if (strcmp(ext, SupportedPackagesExtensions[i]) == 0) {
			return i;
		}
	}

	return -1;
}

const char* _findExtensionInPlace(const char* fileName) {
	const char* result = fileName;

	while (*result != '\0' && *result != '.') {
		result++;
	}

	if (*result == '.')
		result++;

	return result;
}

int32 _ScanForPackagesRecursive(std::vector<std::string>& outPackages, const char* path) {
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

int32 ScanForPackages(std::vector<std::string>& outPackages) {
	WIN32_FIND_DATAA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	int32 result = 0;
	std::string subPath = packagesRootPath;
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

int32 ReleasePackage()
{
	activePackage = nullptr;
	currentStaticMesh = currentTerrain = -1;
	currentStaticActorMesh = -1;

	return 0;
}

UPackage* GetPackage(const char* name)
{
	char message[256];
	UPackage* out = nullptr;

	for (size_t i = 0; i < packages.size(); i++)
	{
		if (!strcmp(packages[i]->Name(), name)) {
			return packages[i];
		}
	}

	std::string fileName = "";
	fileName += name;
	fileName += '.';

	std::string filePath = "";
	for (size_t i = 0; i < allPackagesPaths.size(); i++)
	{
		auto pos = allPackagesPaths[i].find(fileName);

		if (pos == std::string::npos) {
			continue;
		}

		filePath = allPackagesPaths[i].c_str();
	}

	if (filePath == "") {
#if DISPLAY_PACKAGE_ERRORS
		snprintf(message, 256, "Failed to find package name[%s]", name);
		MessageBox(NULL, message, "Error", MB_OK);
#endif
		return nullptr;
	}

	auto package = LoadPackage(filePath.c_str());
	if (!package) {
#if DISPLAY_PACKAGE_ERRORS
		snprintf(message, 256, "Failed to load package name[%s]", name);
		MessageBox(NULL, message, "Error", MB_OK);
#endif

		return nullptr;
	}

	return package;
}

void SetStaticMesh(int32 index)
{
	if (!activePackage && activePackage->staticMeshees.size() <= index) {
		return;
	}

	currentStaticMesh = index;
}

void SetStaticActorMesh(int32 index)
{
	if (!activePackage && activePackage->actorsMeshees.size() <= index) {
		return;
	}

	currentStaticActorMesh = index;
}

void SetTerrain(int32 index)
{
	if (!activePackage && activePackage->terrainTiles.size() <= index) {
		return;
	}

	currentTerrain = index;
}

void ClearStaticMesh() {
	currentStaticMesh = -1;
}

void ClearStaticMeshActor() {
	currentStaticActorMesh = -1;
}

void ClearTerrain() {
	currentTerrain = -1;
}

int32 GetCurrentStaticMeshIndex() {
	return currentStaticMesh;
}

int32 GetCurrentStaticActorMeshIndex() {
	return currentStaticActorMesh;
}

int32 GetCurrentTerrainIndex() {
	return currentTerrain;
}

int32 ExportForSkylake(int version, char* name, char* exportpath,
	int continentId, int worldMapWorldId, int worldMapGuardId, int worldMapSectionId, int clusterCellSize)
{
	if (!DirectoryExists(exportpath)) {
		MessageBoxA(NULL, "Invalid export path!", "Error", MB_OK);
		return 0;
	}

	if (!name || !strcmp(name, "")) {
		MessageBoxA(NULL, "Invalid export file name", "Error", MB_OK);
		return 0;
	}

	std::string path = exportpath;
	path += "//";
	path += name;

	if (FileExists(path.c_str())) {
		MessageBoxA(NULL, "File with that name exists!", "Error", MB_OK);
		return 0;
	}

	FIStream data;

	data.WriteUInt32(version);

	INT64 nextPos = data.NextPos32();

	for (size_t i = 0; i < packages.size(); i++)
	{
		auto temp = data._pos;
		data._pos = nextPos;
		data.WriteUInt32(temp);
		data._pos = temp;

		data.WriteUInt32(data._pos);

		nextPos = data.NextPos32();
		packages[i]->Export(data);

		data.WriteUInt32(continentId);
		data.WriteUInt32(worldMapWorldId);
		data.WriteUInt32(worldMapGuardId);
		data.WriteUInt32(worldMapSectionId);
		data.WriteUInt32(clusterCellSize);

		break;
	}

	data.save_to_file(path.c_str());

	MessageBoxA(NULL, "Exported!", "Info", MB_OK);
	return 0;
}

void PackageManagerRender() {
	if (!activePackage) {
		return;
	}

	for (size_t i = 0; i < packages.size(); i++)
	{
		if (packages[i]->levels.size()) {
			packages[i]->levels[0]->Render();
		}

		for (size_t j = 0; j < packages[i]->levels.size(); j++)
		{
			packages[i]->levels[j]->Render();
		}
	}


	if (currentTerrain != -1) {
		activePackage->terrainTiles[currentTerrain]->Render();
	}

	if (currentStaticMesh != -1) {
		((StaticMesh*)activePackage->staticMeshees[currentStaticMesh])->Render();
	}

	if (currentStaticActorMesh != -1) {
		activePackage->actorsMeshees[currentStaticActorMesh]->Render();
	}

	if (activePackage->levels.size() &&
		activePackage->levels[0]->_drawing) {

		activePackage->levels[0]->Render();
	}
}

ActorsExportEntry::ActorsExportEntry(StaticMeshActor* a)
{
	actor = a;
	vOffset = 0;
	vCount = a->vertexCount;
}

ActorsExportEntry::ActorsExportEntry(Terrain* a)
{
	actor = nullptr;
	terrain = a;
	vOffset = 0;
	vCount = a->vertexCount;
}

ActorsExportEntry::ActorsExportEntry(TerrainComponent* a)
{
	actor = nullptr;
	terrain = nullptr;
	component = a;
	vOffset = 0;
	vCount = a->vertexCount;
}

void ActorsExportEntry::WriteVertices(std::ofstream& file, uint32 offset)
{
	vOffset = offset;

	char lineBuffer[256];

	//vertices
	if (actor) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = actor->vertices + i;
			snprintf(lineBuffer, 256, "v %f %f %f\n", v->position.x, v->position.y, v->position.z);
			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
	else if (terrain) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = terrain->vertices + i;
			snprintf(lineBuffer, 256, "v %f %f %f\n", v->position.x, v->position.y, v->position.z);
			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
	else if (component) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = component->vertices + i;
			snprintf(lineBuffer, 256, "v %f %f %f\n", v->position.x, v->position.y, v->position.z);
			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
}

void ActorsExportEntry::WriteNormals(std::ofstream& file)
{
	char lineBuffer[256];

	//normal
	if (actor) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = actor->vertices + i;
			snprintf(lineBuffer, 256, "vn %f %f %f\n", v->normal.x, v->normal.y, v->normal.z);
			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
	else if (terrain) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = terrain->vertices + i;
			snprintf(lineBuffer, 256, "vn %f %f %f\n", v->normal.x, v->normal.y, v->normal.z);
			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
	else if (component) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = component->vertices + i;
			snprintf(lineBuffer, 256, "vn %f %f %f\n", v->normal.x, v->normal.y, v->normal.z);
			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
}

void ActorsExportEntry::WriteTexCoods(std::ofstream& file)
{
	char lineBuffer[256];

	//text coords
	if (actor) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = actor->vertices + i;
			snprintf(lineBuffer, 256, "vt %f %f\n", v->uv.x, v->uv.y);
			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
	else if (terrain) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = terrain->vertices + i;
			snprintf(lineBuffer, 256, "vt %f %f\n", v->uv.x, v->uv.y);
			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
	else if (component) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = component->vertices + i;
			snprintf(lineBuffer, 256, "vt %f %f\n", v->uv.x, v->uv.y);
			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
}

void ActorsExportEntry::WriteIndices(std::ofstream& file)
{
	char lineBuffer[256];

	if (actor)
	{
		auto* mesh = actor->mesh;
		if (!mesh || !mesh->lodInfo) {
			throw std::exception("Actor with no mesh!!");
		}

		auto* lod = mesh->lodInfo + 0;
		auto faceCont = lod->indexContainer->elementCount / 3;
		auto* lodIndices = (uint16*)lod->indexContainer->rawData;

		uint32 faceCount = actor->mesh->lodInfo[0].indexContainer->elementCount / 3;
		//faces
		for (uint32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
			snprintf(lineBuffer, 256, "f %d %d %d\n",
				lodIndices[faceIndex * 3 + 2] + 1 + vOffset,
				lodIndices[faceIndex * 3 + 1] + 1 + vOffset,
				lodIndices[faceIndex * 3 + 0] + 1 + vOffset);

			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
	else if (terrain) {
		uint32 faceCount = terrain->indexCount / 3;

		for (uint32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
			snprintf(lineBuffer, 256, "f %d %d %d\n",
				terrain->indices[faceIndex * 3 + 2] + 1 + vOffset,
				terrain->indices[faceIndex * 3 + 1] + 1 + vOffset,
				terrain->indices[faceIndex * 3 + 0] + 1 + vOffset);

			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
	else if (component) {
		uint32 faceCount = component->indexCount / 3;

		for (uint32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
			snprintf(lineBuffer, 256, "f %d %d %d\n",
				component->indices[faceIndex * 3 + 2] + 1 + vOffset,
				component->indices[faceIndex * 3 + 1] + 1 + vOffset,
				component->indices[faceIndex * 3 + 0] + 1 + vOffset);

			size_t lineSize = strnlen_s(lineBuffer, 256);
			file.write(lineBuffer, lineSize);
		}
	}
}

void ActorsExportEntry::AppendVerts(std::vector<D3DXVECTOR3>& verts, uint32 offset)
{
	vOffset = offset;

	//vertices
	if (actor) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = actor->vertices + i;
			verts.push_back(v->position);
		}
	}
	else if (terrain) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = terrain->vertices + i;
			verts.push_back(v->position);
		}
	}
	else if (component) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = component->vertices + i;
			verts.push_back(v->position);
		}
	}
}

void ActorsExportEntry::AppendNormals(std::vector<D3DXVECTOR3>& normals)
{
	//vertices
	if (actor) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = actor->vertices + i;
			normals.push_back(v->normal);
		}
	}
	else if (terrain) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = terrain->vertices + i;
			normals.push_back(v->normal);
		}
	}
	else if (component) {
		for (size_t i = 0; i < vCount; i++)
		{
			D3DVertex* v = component->vertices + i;
			normals.push_back(v->normal);
		}
	}
}

void ActorsExportEntry::AppendIndices(std::vector<int>& indices)
{
	if (actor)
	{
		auto* mesh = actor->mesh;
		if (!mesh || !mesh->lodInfo) {
			throw std::exception("Actor with no mesh!!");
		}

		auto* lod = mesh->lodInfo + 0;
		auto faceCont = lod->indexContainer->elementCount / 3;
		auto* lodIndices = (uint16*)lod->indexContainer->rawData;

		uint32 faceCount = actor->mesh->lodInfo[0].indexContainer->elementCount / 3;
		//faces
		for (uint32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
			indices.push_back(lodIndices[faceIndex * 3 + 2] + 1 + vOffset);
			indices.push_back(lodIndices[faceIndex * 3 + 1] + 1 + vOffset);
			indices.push_back(lodIndices[faceIndex * 3 + 0] + 1 + vOffset);
		}
	}
	else if (terrain) {
		uint32 faceCount = terrain->indexCount / 3;

		for (uint32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
			indices.push_back(terrain->indices[faceIndex * 3 + 2] + 1 + vOffset);
			indices.push_back(terrain->indices[faceIndex * 3 + 1] + 1 + vOffset);
			indices.push_back(terrain->indices[faceIndex * 3 + 0] + 1 + vOffset);
		}
	}
	else if (component) {
		uint32 faceCount = component->indexCount / 3;

		for (uint32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
			indices.push_back(component->indices[faceIndex * 3 + 2] + 1 + vOffset);
			indices.push_back(component->indices[faceIndex * 3 + 1] + 1 + vOffset);
			indices.push_back(component->indices[faceIndex * 3 + 0] + 1 + vOffset);
		}
	}
}
