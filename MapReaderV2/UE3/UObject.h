#ifndef UOBJECT_H
#define UOBJECT_H

#include <vector>

#include "FReadable.h"

#include <D3D11.h>
#include <D3DX10math.h>

extern ID3D11Device* GetD3DDevice();
extern ID3D11DeviceContext* GetD3DDeviceContext();

struct FPropertyTag;
struct FStateFrame;

struct UObject : FReadable {
	UObject * native{ nullptr };

	int32 netIndex{ 0 };
	int32 expressionIndex{ 0 };
	uint32 dataSize{ 0 };
	uint32 rawDataOffset{ 0 };

	FStateFrame * stateFrame;

	FObject * fObject{ nullptr };
	FObjectExport * exportObject{ nullptr };
	FObjectImport* importObject{ nullptr };

	std::vector<FPropertyTag*> properties;

	const char* Name() const;
	const char* Class() const;

	static UObject * ReadFromStream(FIStream * stream);
	static UObject* UObjectForClass(const char* uClass);

	UObject();
	UObject(UPackage* package);
	UObject(FIStream* package);
	~UObject();
	void ReadObject(bool fallBackToSafe);
	const char* Path();
	FObject * Parent() const;

	virtual const bool IsNative() const;
	virtual const bool IsZero() const;

	virtual void Init (){}

	FPropertyTag  * GetPropByName(const char* name) const;
	virtual void ReadData(FIStream & data);

	virtual void Render() {}

private:
	std::string _path;

};

#endif

