#ifndef FPROPERTYTAG_H
#define FPROPERTYTAG_H

#include "FReadable.h"
#include "FString.h"

#include <string>
#include <map>
#include <vector>

struct UObject;

struct FPropertyTag : FReadable {
	std::string arrayType;

	FName fName;
	FName fType;
	FName fEnum;
	FName fStruct;
	int32 dataSize;
	int32 arrayIndex;
	int32 elementCount;

	void * value{ nullptr };

	UObject * uObject{ nullptr };
	FPropertyTag * parent{ nullptr };

	FPropertyTag(FIStream * stream, UObject* uObj, FPropertyTag* parent);
	~FPropertyTag();

	const char* Name() const;
	const char* Type() const;
	const char* StructName() const;
	const bool IsNone() const;

	static std::map<const char *, const char*> structs;
	static std::map<const char *, std::vector<std::string>>classes;
	static std::map<const char *, std::map< const char*, const char*>> properties;
	static void InitStructs();
	static void InitProperties();
	static void InitClasses();
	static const  char * propertyTypeForName(const char* name, const  char*_class);

};

#endif
