#include "Level.h"

#include "Actor.h"
#include "Terrain.h"

FKCachedConvexDataElement::FKCachedConvexDataElement()
{
	convexElementData = nullptr;
}

FKCachedConvexDataElement::FKCachedConvexDataElement(FIStream * stream)
	:FReadable(stream)
{
	convexElementData = new FByteArray(stream);
}

FKCachedConvexDataElement::~FKCachedConvexDataElement()
{
	if (convexElementData) {
		delete convexElementData;
		convexElementData = nullptr;
	}
}

FKCachedPerTriData::FKCachedPerTriData()
{
}


FKCachedPerTriData::FKCachedPerTriData(FIStream * stream)
	:FReadable(stream)
{
	cachedPerTriData = FByteArray(stream);
}

FKCachedConvexData::FKCachedConvexData()
{
	cachedConvexElements = nullptr;
}

FKCachedConvexData::FKCachedConvexData(FIStream * stream)
	:FReadable(stream)
{
	cachedConvexElements = new FPArray<FKCachedConvexDataElement>(stream);
}

FKCachedConvexData::~FKCachedConvexData()
{
	if (cachedConvexElements) {
		delete cachedConvexElements;
		cachedConvexElements = nullptr;
	}
}

FCachedPhysSMData::FCachedPhysSMData()
{
}

FCachedPhysSMData::FCachedPhysSMData(FIStream * stream)
	:FReadable(stream)
{
	scale3D = FVector3(stream);
	cachedDataIndex = stream->ReadInt32();
}

FCachedPerTriPhysSMData::FCachedPerTriPhysSMData()
{
}

FCachedPerTriPhysSMData::FCachedPerTriPhysSMData(FIStream * stream)
	:FReadable(stream)
{

}

void Level::ReadData(FIStream & data)
{
	UObject::ReadData(data);

	data._pos = rawDataOffset;
	FIStream* stream = &data;

	actors = new TransUObjectArray(stream);
	url = new FUrl(stream);
	model = UObject::ReadFromStream(stream);
	/*if (model) {
		int32 tempPos = data._pos;
		model->ReadData(data);
		data._pos = tempPos;
	}*/

	return;
	modelComponents = UObjectArray(stream);
	gameSequences = UObjectArray(stream);
	textureToInstancesMap = PMapUObjectKeyArray<FStreamableTextureInstance>(stream);

	cachedPhysBSPData = FByteArray(stream);
	cachedPhysSMDataMap = FMultiMap<FCachedPhysSMData>(stream);
	cachedPhysSMDataStore = new FPArray<FKCachedConvexData>(stream);

	cachedPhysPerTriSMDataMap = FMultiMap<FCachedPerTriPhysSMData>(stream);
	cachedPhysPerTriSMDataStore = FPArray<FKCachedPerTriData>(stream);

	cachedPhysBSPDataVersion = data.ReadInt32();
	cachedPhysSMDataVersion = data.ReadInt32();

	forceStreamTextures = FMap<Texture2D*, int32>(stream);

	navListStart = UObject::ReadFromStream(stream);
	navListEnd = UObject::ReadFromStream(stream);

	coverListStart = UObject::ReadFromStream(stream);
	coverListEnd = UObject::ReadFromStream(stream);

	/*pylonListStart = UObject::ReadFromStream(stream);
	  pylonListEnd= UObject::ReadFromStream(stream);*/

	crossLevelActors = FArray<UObject*>(stream);
	unk = data.ReadInt32();

	if (exportObject->originalOffset + exportObject->serialSize != data._pos) {
		throw std::exception("unexpected data found");
	}
}

void Level::Init() {
	for (size_t i = 0; i < actors->count; i++) {
		if (actors->data[i]) {
			if (!strcmp(actors->data[i]->Name(), "StaticMeshActor")) {
				StaticMeshActor* actor = (StaticMeshActor*)actors->data[i];
				actor->ReadObject(false);
			}
			else if (!strcmp(actors->data[i]->Name(), "Terrain")) {
				terrains.push_back((Terrain*)actors->data[i]);
			}
		}
	}

	package->levels.push_back(this);
}

void Level::Render()
{
	if (!actors) {
		return;
	}

	for (size_t i = 0; i < actors->count; i++)
	{
		if (!actors->data[i]) {
			continue;
		}

		if (strcmp(actors->data[i]->Name(), "StaticMeshActor") == 0 ||
			strcmp(actors->data[i]->Name(), "Terrain") == 0) {
			actors->data[i]->Render();
		}
	}
}

void Level::Export(FIStream & data)
{
	if (!terrains.size()) {
		data.WriteUInt16(0);
		return;
	}

	data.WriteUInt16(1);
	Terrain * t = terrains[0];
	t->Export(data);
}

Level::Level()
	:UObject()
{
	_drawing = false;
	url = nullptr;
	cachedPhysSMDataStore = nullptr;
	actors = nullptr;
}
Level::~Level()
{
	if (url) {
		delete url;
		url = nullptr;
	}

	if (cachedPhysSMDataStore) {
		delete cachedPhysSMDataStore;
		cachedPhysSMDataStore = nullptr;
	}

	if (actors) {
		delete actors;
		actors = nullptr;
	}
}
