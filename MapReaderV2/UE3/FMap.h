#ifndef FMAP_H
#define FMAP_H

#include "FReadable.h"
#include "FArray.h"
#include "FIStream.h"

#include "UPackage.h"
#include "UObject.h"

#include <map>

struct FIStream;
struct UObject;
struct Texture2D;

template<typename K, typename T>
struct FMap : FReadable {

	std::map<K, T> keys;

	FMap();
	FMap(FIStream* stream);
	~FMap();
};

template<typename K, typename T>
inline FMap<K, T>::FMap()
{
}

template<typename K, typename T>
inline FMap<K, T>::FMap(FIStream * stream)
	:FReadable(stream) {

	int32 count = stream->ReadInt32();

	for (size_t i = 0; i < count; i++)
	{
		K key = K(stream);

		T obj = T(stream);

		keys.insert(std::pair<K, T>(key, obj));
	}
}

template<typename K, typename T>
inline FMap<K, T>::~FMap()
{
}

template<class T>
struct FMultiMap : FReadable {

	std::map<int32, std::vector<T>> keys;

	FMultiMap() {}
	FMultiMap(FIStream* stream);
};

template<class T>
inline FMultiMap<T>::FMultiMap(FIStream * stream)
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
			keys[objectIndex] = std::vector<T>();
		}

		keys[objectIndex].push_back(T(stream));
	}
}

template <typename T, typename K>
struct _pair {
	T * key;
	K * value;
};

template<typename T>
struct FMapUObjectKey :FReadable {

	_pair<UObject, T> * data;
	uint32 count;

	FMapUObjectKey();
	FMapUObjectKey(FIStream * stream);
	~FMapUObjectKey();
};


template<typename T>
inline FMapUObjectKey<T>::FMapUObjectKey()
{
	data = nullptr;
	count = 0;
}

template<typename T>
inline FMapUObjectKey<T>::FMapUObjectKey(FIStream * stream)
{
	count = stream->ReadInt32();
	if (count) {
		data = new _pair<UObject, T>[count];

		for (size_t i = 0; i < count; i++)
		{
			data[i].key = UObject::ReadFromStream(stream);
			data[i].value = new T(stream);
		}
	}
	else {
		data = nullptr;
	}
}

template<typename T>
inline FMapUObjectKey<T>::~FMapUObjectKey()
{
	if (data) {
		for (size_t i = 0; i < count; i++)
		{
			delete data[i].value;
		}

		delete[] data;
		data = nullptr;
	}
}



template<typename T, typename K>
struct __pairArray {
	T* key;
	uint32 count;
	K * array_value;
};

template<typename T>
struct PMapUObjectKeyArray{
	__pairArray<UObject, T> * data;
	uint32 count;

	PMapUObjectKeyArray();
	PMapUObjectKeyArray(FIStream * stream);
	~PMapUObjectKeyArray();
};


template<typename T>
inline PMapUObjectKeyArray<T>::PMapUObjectKeyArray()
{
	data = nullptr;
	count = 0;
}

template<typename T>
inline PMapUObjectKeyArray<T>::PMapUObjectKeyArray(FIStream * stream)
{
	count = stream->ReadInt32();
	if (count) {
		data = new __pairArray<UObject, T>[count];

		for (size_t i = 0; i < count; i++)
		{
			data[i].key = UObject::ReadFromStream(stream);
			data[i].count = stream->ReadUInt32();
			if (data[i].count) {
				data[i].array_value = new T[data[i].count];
				for (size_t j = 0; j < data[i].count; j++)
				{
					data[i].array_value[j] = T(stream);
				}
			}
			else {
				data[i].array_value = nullptr;
			}
		}
	}
	else {
		data = nullptr;
	}
}

template<typename T>
inline PMapUObjectKeyArray<T>::~PMapUObjectKeyArray()
{
	if (data) {
		for (size_t i = 0; i < count; i++)
		{
			delete[] data[i].array_value;
		}

		delete[] data;
		data = nullptr;
	}
}





#endif
