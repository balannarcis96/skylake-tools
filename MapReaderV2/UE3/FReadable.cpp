#include "FReadable.h"
#include "FIStream.h"
#include "UPackage.h"
#include "UObject.h"
#include "FArray.h"
#include "FString.h"
#include "FGUID.h"
#include "CustomFArrays.h"

#include <Windows.h>

FReadable::FReadable()
{
	bIsInt = 0;
	package = nullptr;
	objectIndex = 0;
	intVal = 0;
}

FReadable::FReadable(UPackage * package)
{
	this->package = package;
}

FReadable::FReadable(int32 val)
{
	intVal = val;
	bIsInt = 1;
	package = nullptr;
}

FReadable::FReadable(FIStream * stream)
{
	bIsInt = 0;

	package = stream->_package;
}

FObject::FObject()
{ }

FObject::FObject(int32 type) : FReadable(), fObjectType(type)
{}

FObject::FObject(UPackage * package, int32 type)
	: FReadable(package), fObjectType(type)
{}

FObject::FObject(FIStream * stream, int32 type)
	: FReadable(stream), fObjectType(type), uObject{ nullptr }, nameIndex{ 0 }, classObjectIndex{ 0 }
{ }

FObject::~FObject()
{
	if (uObject && uObject->package == package) {
		delete uObject;
		uObject = nullptr;
	}
}

const char * FObject::Class() const
{
	if (classObjectIndex == 0) {
		return "UClass";
	}

	UObject *uObject = package->uObjectForIndex(classObjectIndex);
	if (uObject) {
		return uObject->Name();
	}

	throw std::exception("Unknown ClassName");
}

const char * FObject::Name() const
{
	return package->nameForIndex(nameIndex);
}

FObject * FObject::Parent() const
{
	return package->fObjectForIndex(parentIndex);
}

void FObject::AddChild(FObject * obj)
{
	children.push_back(obj);
}

void FObject::RemoveChild(FObject * obj)
{
	int32 index = -1;
	for (int32 i = 0; i < children.size(); i++)
	{
		if (children[i] == obj) {
			index = i;
			break;
		}
	}

	if (index >= 0) {
		children.erase(children.begin() + index);
	}
}

void FObject::Cleanup()
{
	children.clear();
}

FObjectExport::FObjectExport()
{
	generationNetObjectCount = nullptr;
	packageGuid = nullptr;
}

FObjectExport::FObjectExport(UPackage * package)
	:FObject(package, FT_FObjectExport)
{
	generationNetObjectCount = nullptr;
	packageGuid = nullptr;
}

FObjectExport::~FObjectExport()
{
	if (packageGuid) {
		delete packageGuid;
		packageGuid = nullptr;
	}

	if (generationNetObjectCount) {
		delete generationNetObjectCount;
		generationNetObjectCount = nullptr;
	}
}

FObjectExport::FObjectExport(FIStream * stream)
	: FObject(stream, FT_FObjectExport)
{
	classObjectIndex = stream->ReadInt32();
	superObjectIndex = stream->ReadInt32();
	packageIndex = stream->ReadInt32();
	objectName = new FName(stream);

	archetypeIndex = stream->ReadInt32();
	objectFlags = (EObjectFlags)stream->ReadInt64();
	serialSize = stream->ReadUInt32();
	if (serialSize > 0)
	{
		serialOffset = stream->ReadUInt32();
		originalOffset = serialOffset;
	}

	exportFlags = (EFExportFlags)stream->ReadInt32();
	generationNetObjectCount = new FIntArray(stream);
	packageGuid = new FGUID(stream);
	packageFlags = (EPackageFlags)stream->ReadInt32();
}

void FObjectExport::serialize()
{
	if (uObject) {
		return;
	}

	if (strE(Class(), "StaticMeshActor")) {
		printf("asdasd");
	}

	UObject * obj = UObject::UObjectForClass(Class());
	uObject = obj;
	obj->package = package;
	obj->exportObject = this;
	obj->importObject = nullptr;

	if (parentIndex) {
		FObjectExport * parent = (FObjectExport*)package->fObjectForIndex(parentIndex);
		if (!parent->uObject) {
			parent->serialize();
		}

		parent->children.push_back(this);
	}
}

const char * FObjectExport::Path()
{
	if (_path != "") {
		return _path.c_str();
	}

	_path = Name();

	FObject * parent = Parent();
	FObject* last = nullptr;
	while (parent) {
		_path.insert(0, "/");
		_path.insert(0, parent->Name());

		last = parent;
		parent = parent->Parent();
	}

	FObjectExport * expo = (FObjectExport*)last;
	if (expo && !(exportFlags & EF_ForcedExport)) {
		_path.insert(0, "/");
		_path.insert(0, package->Name());
	}

	return _path.c_str();
}

FObjectRef::FObjectRef()
{
}

FObjectRef::FObjectRef(FIStream * stream)
	:FReadable(stream)
{
	value = stream->ReadInt32();
}

FObjectImport::FObjectImport()
	: FObject(FT_FObjectImport)
{
	ClassPackage = nullptr;
	ClassName = nullptr;
	objectName = nullptr;
}

FObjectImport::~FObjectImport()
{
	if (ClassPackage) {
		delete ClassPackage;
		ClassPackage = nullptr;
	}

	if (ClassName) {
		delete ClassName;
		ClassName = nullptr;
	}

	if (objectName) {
		delete objectName;
		objectName = nullptr;
	}
}


void FObjectImport::serialize()
{
	if (uObject) {
		return;
	}

	uObject = new UObject(package);
	uObject->importObject = this;

	if (parentIndex) {
		FObjectExport * parent = (FObjectExport*)package->fObjectForIndex(parentIndex);
		if (!parent->uObject) {
			parent->serialize();
		}

		parent->children.push_back(this);
	}
}

const char * FObjectImport::Class() const
{
	return ClassName->Name();
}

const char * FObjectImport::PackageClassName() const
{
	return ClassPackage->Name();
}

const char * FObjectImport::Path()
{
	if (_path != "") {
		return _path.c_str();
	}

	const char* name = Name();

	_path = std::string(name);

	FObject * parent = Parent();
	FObject* last = nullptr;
	while (parent) {
		_path.insert(0, "/");
		_path.insert(0, parent->Name());

		last = parent;
		parent = parent->Parent();
	}

	FObjectExport * expo = (FObjectExport*)last;
	if (expo) {
		_path.insert(0, "/");
		_path.insert(0, package->Name());
	}

	return _path.c_str();
}

void ReadUObject(UObject ** uObj, FIStream * stream)
{
	*uObj = UObject::ReadFromStream(stream);
}
