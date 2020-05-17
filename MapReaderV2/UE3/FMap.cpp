#include "FMap.h"

#include "Texture2D.h"
#include "FStreamableTextureInstance.h"

template<>
inline FMap<FName*, UObject*>::~FMap()
{
	for (auto&& p : keys) {
		delete p.first;
		delete p.second;
	}

	keys.clear();
}

template<>
inline FMap<FName*, UObject*>::FMap(FIStream * stream)
	:FReadable(stream) {

	int32 count = stream->ReadInt32();

	for (size_t i = 0; i < count; i++)
	{
		FName * name = new FName(stream);
		UObject * obj = UObject::ReadFromStream(stream);

		keys.insert(std::pair<FName*, UObject*>(name, obj));
	}
}

template<>
inline FMap<Texture2D*, FStreamableTextureInstance>::FMap(FIStream * stream)
	:FReadable(stream) {

	int32 count = stream->ReadInt32();

	for (size_t i = 0; i < count; i++)
	{
		Texture2D * texture = (Texture2D *)UObject::ReadFromStream(stream);
		keys.insert(std::pair<Texture2D*, FStreamableTextureInstance>(texture, FStreamableTextureInstance(stream)));
	}
}

template<>
inline FMap<Texture2D*, int32>::FMap(FIStream * stream)
	:FReadable(stream) {

	int32 count = stream->ReadInt32();
	for (size_t i = 0; i < count; i++)
	{
		Texture2D * texture = (Texture2D *)UObject::ReadFromStream(stream);
		keys.insert(std::pair<Texture2D*, int32>(texture, stream->ReadInt32()));
	}
}

template<>
inline FMultiMap<UObject*>::FMultiMap(FIStream * stream)
	:FReadable(stream) {

	int32 count = stream->ReadInt32();

	for (size_t i = 0; i < count; i++)
	{
		int32 objectIndex = stream->ReadInt32();
		UObject * obj = package->uObjectForIndex(objectIndex);
		if (!obj) {
			throw "Failed to resolve multimap ";
		}

		if (keys.find(objectIndex) == keys.end()) {
			keys[objectIndex] = std::vector<UObject*>();
		}

		keys[objectIndex].push_back(UObject::ReadFromStream(stream));
	}
}
