#ifndef FREADABLE_H
#define FREADABLE_H

#include <vector>
#include <string>

#include "typeDefs.h"
#include "Enums.h"


struct UPackage;
struct FIStream;
struct UObject;
struct FGUID;
struct FIntArray;
struct FName;

struct FReadable {
	int32 objectIndex;

	int32 intVal;
	int32 bIsInt;

	UPackage * package;

	FReadable();
	FReadable(UPackage * package);
	FReadable(int32);
	FReadable(FIStream * stream);
};

struct FObject : FReadable {
	int32 fObjectType{ FT_FObject };

	FName * objectName = nullptr;
	int32 nameIndex{ 0 };
	int32 classObjectIndex{ 0 };
	int32 parentIndex{ 0 };

	UObject * uObject{ nullptr };

	int32 isExpanded{ 0 };
	std::vector<FObject*> children;

	FObject();
	FObject(int32 type);
	FObject(UPackage * package, int32 type);
	FObject(FIStream *stream, int32 type);
	~FObject();
	virtual  const char* Class() const;
	virtual  const char* Name() const;
	virtual FObject* Parent() const;
	virtual void AddChild(FObject * obj);
	virtual void RemoveChild(FObject * obj);
	virtual void Cleanup();

protected:
	std::string _path;
};

struct FObjectExport : FObject {
	int32 packageIndex;
	int32 superObjectIndex;
	EObjectFlags objectFlags;
	uint32 serialSize;
	uint32 serialOffset;
	uint32 originalOffset;
	EFExportFlags exportFlags;
	FIntArray* generationNetObjectCount = nullptr;
	FGUID*  packageGuid = nullptr;
	int32 archetypeIndex ;
	EPackageFlags packageFlags;

	FObjectExport();
	FObjectExport(UPackage * package);
	FObjectExport(FIStream * stream);
	~FObjectExport();

	void serialize();
	const char * Path();
};

struct FObjectImport : FObject {
	FName * ClassPackage;
	FName * ClassName;
	int32 ParentIndex;
	bool resolved = false;

	FObjectImport();
	~FObjectImport();

	void serialize();
	const char* Class() const;
	const char * PackageClassName() const;
	const char * Path();
};

struct FObjectRef : FReadable {
	int32      value;

	FObjectRef();
	FObjectRef(FIStream * stream);
};


void ReadUObject(UObject ** uObj, FIStream * stream);
#endif