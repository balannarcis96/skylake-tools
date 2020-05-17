#ifndef FARRAY_H
#define FARRAY_H

#include "FIStream.h"
#include "FReadable.h"
#include "typeDefs.h"

#include <vector>

struct UObject;
struct UState;

UObject * __readUObject(FIStream* stream);

template<class T>
struct FArray : FReadable
{
	T* data = nullptr;
	int count = 0;

	void Clear();

	FArray();
	virtual ~FArray();

	FArray(FIStream * stream);

	T * operator[](int index) {
		if (index < 0 || index >= count) {
			return nullptr;
		}

		return data[index];
	}

protected:
	virtual void _init(FIStream * stream);
};

template<class T>
inline void FArray<T>::Clear()
{
	if (data) {
		delete[] data;
		data = nullptr;
	}

	count = 0;
}

template<class T>
inline FArray<T>::FArray()
{
	data = nullptr;
	count = 0;
}

template<class T>
inline FArray<T>::FArray(FIStream * stream)
{
	_init(stream);

	count = stream->ReadInt32();

	if (count) {
		data = new T[count];

		for (size_t i = 0; i < count; i++)
		{
			data[i] = T(stream);
		}
	}
	else
	{
		data = nullptr;
	}
}

template<class T>
inline void FArray<T>::_init(FIStream * stream)
{
}

template<class T>
inline FArray<T>::~FArray()
{
	if (data) {
		delete[] data;
		data = nullptr;
	}
}


template<class T>
struct TransFArray : FArray<T> {
	UObject *  owner;

	TransFArray();
	TransFArray(FIStream* stream);

protected:
	void _init(FIStream * stream);
};


template<class T>
inline TransFArray<T>::TransFArray() {
	owner = nullptr;
}

template<class T>
inline TransFArray<T>::TransFArray(FIStream * stream)
	:FArray<T>(stream)
{
}

template<class T>
inline void TransFArray<T>::_init(FIStream * stream)
{
	owner = __readUObject(stream);
}

struct TransUObjectArray :FReadable {
	UObject * owner;
	UObject ** data;
	uint32 count;

	TransUObjectArray();
	TransUObjectArray(FIStream * stream);
	~TransUObjectArray();

	const UObject * operator[](int32 index) const {
		if (index < 0 || index >= count) {
			return nullptr;
		}

		return data[index];
	}
};


struct UObjectArray :FReadable {
	UObject ** data;
	uint32 count;

	UObjectArray();
	UObjectArray(FIStream * stream);
	~UObjectArray();

	const UObject * operator[](int32 index) const {
		if (index < 0 || index >= count) {
			return nullptr;
		}

		return data[index];
	}
};


struct FByteArray : FReadable {
	uint32 count;
	uint32 elementSize;
	uint8* data;

	FByteArray();
	FByteArray(FIStream * stream);
	~FByteArray();
};

template<typename T>
struct FPArray {
	uint32 count;
	T**data;

	FPArray();
	FPArray(FIStream *stream);
	~FPArray();

	const T* operator[](int32 index) const {
		if (index < 0 || index >= count) {
			return nullptr;
		}

		return data[index];
	}
};


template<typename T>
inline FPArray<T>::FPArray()
{
	data = nullptr;
	count = 0;
}

template<typename T>
inline FPArray<T>::FPArray(FIStream * stream)
{
	count = stream->ReadInt32();
	if (count) {
		data = new T*[count];

		for (size_t i = 0; i < count; i++)
		{
			data[i] = new T(stream);
		}
	}
	else {
		data = nullptr;
	}
}

template<typename T>
inline FPArray<T>::~FPArray()
{
	if (data) {
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

#endif
