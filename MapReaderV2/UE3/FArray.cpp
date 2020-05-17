#include "FArray.h"
#include "UPackage.h"
#include "FString.h"
#include "UObject.h"

template<>
inline FArray<uint8>::FArray(FIStream * stream)
{
	count = stream->ReadInt32();

	if (count) {
		data = new uint8[count];
		stream->Read(data, count);
	}
	else
	{
		data = nullptr;
	}
}

template<>
inline FArray<FGeneration*>::FArray(FIStream * stream)
{
	count = stream->ReadInt32();

	if (count) {
		data = new FGeneration*[count];
		for (size_t i = 0; i < count; i++)
		{
			data[i] = new FGeneration(stream);
		}
	}
	else
	{
		data = nullptr;
	}
}

template<>
inline FArray<FGeneration*>::~FArray()
{
	if (count) {
		for (size_t i = 0; i < count; i++)
		{
			if (data[i]) {
				delete data[i];
				data[i] = nullptr;
			}
		}

		delete[] data;
		data = nullptr;
	}
}

template<>
inline FArray<FString*>::FArray(FIStream * stream) {
	count = stream->ReadInt32();

	if (count) {
		data = new FString*[count];
		for (size_t i = 0; i < count; i++)
		{
			data[i] = new FString(stream);
		}
	}
	else
	{
		data = nullptr;
	}
}

template<>
inline FArray<FString*>::~FArray() {
	if (data) {
		for (size_t i = 0; i < count; i++) {
			delete data[i];
		}
	}

	delete[] data;
	data = nullptr;
}


template<>
inline FArray<UState*>::FArray(FIStream * stream)
{
	count = stream->ReadInt32();

	if (count) {
		data = new UState*[count];
		for (size_t i = 0; i < count; i++)
		{
			data[i] = (UState*)UObject::ReadFromStream(stream);
		}
	}
	else
	{
		data = nullptr;
	}
}

template<>
inline FArray<UObject*>::FArray(FIStream * stream)
{
	count = stream->ReadInt32();
	if (count) {
		data = new UObject*[count];

		for (int32 i = 0; i < count; i++)
		{
			data[i] = UObject::ReadFromStream(stream);
		}
	}
	else
	{
		data = nullptr;
	}
}

UObject * __readUObject(FIStream * stream)
{
	return  UObject::ReadFromStream(stream);
}

TransUObjectArray::TransUObjectArray()
	:FReadable()
{
	owner = nullptr;
	data = nullptr;
}

TransUObjectArray::TransUObjectArray(FIStream * stream)
	:FReadable(stream)
{
	owner = UObject::ReadFromStream(stream);

	count = stream->ReadInt32();
	if (count) {
		data = new UObject*[count];

		for (uint32 i = 0; i < count; i++)
		{
			data[i] = UObject::ReadFromStream(stream);
		}
	}
	else {
		data = nullptr;
	}
}

TransUObjectArray::~TransUObjectArray()
{
	if (owner) {
		owner = nullptr;
	}

	if (data) {
		delete[] data;
		data = nullptr;
	}

	count = 0;
}

UObjectArray::UObjectArray()
{
	data = nullptr;
	count = 0;
}

UObjectArray::UObjectArray(FIStream * stream)
{
	count = stream->ReadUInt32();

	if (count) {
		data = new UObject *[count];

		for (size_t i = 0; i < count; i++)
		{
			data[i] = UObject::ReadFromStream(stream);
		}
	}
	else {
		data = nullptr;
	}
}

UObjectArray::~UObjectArray()
{
	if (data) {
		delete[] data;
		data = nullptr;
	}
}

FByteArray::FByteArray()
	:FReadable()
{
	data = nullptr;
}

FByteArray::FByteArray(FIStream * stream)
	: FReadable(stream)
{
	elementSize = stream->ReadInt32();
	count = stream->ReadInt32();

	uint32 total = elementSize * count;
	if (total) {
		data = new uint8[total];
		stream->Read(data, total);
	}
	else {
		data = nullptr;
	}
}

FByteArray::~FByteArray()
{
	if (data) {
		delete[] data;
		data = nullptr;
	}
}
