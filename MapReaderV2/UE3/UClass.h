#ifndef UCLASS_H
#define UCLASS_H

#include "UObject.h"
#include "TextBuffer.h"
#include "FMap.h"

struct FImplementedInterface : FReadable {
	UObject * objectClass;
	UObject * pointerProperty;

	FImplementedInterface();
	FImplementedInterface(FIStream * stream);
};

struct UField : UObject {
	UField * superField, *next;

	const bool IsNative() const;

	UField(FIStream * stream);
};

struct UStruct : UField {
	TextBuffer *cppText;
	TextBuffer *scriptText;
	UField * children;
	int32 line;
	int32 textPos;
	uint8* scriptData;

	UStruct(FIStream * stream);
	~UStruct();
};

struct UState : UStruct {
	int32 unk;
	uint32 probeMask;
	uint32 stateFlags;
	int16 labelTableOffset;
	u_int64 ignoreMask;
	FMap<FName*, UObject*>* funcMap;

	UState(FIStream * stream);
	~UState();
};

struct UClass : UState {
	uint32 classFlags;
	int32 classUnique;
	UClass * classWithin;
	FName classConfigName;
	FMap<FName*, UObject*>* componentNameToDefaultObjectMap;
	FArray<FImplementedInterface> interferences;
	FArray<FName> hideCategories;
	FArray<FName> autoExpandedCategories;
	UObject* classDefaultObject;

	UClass(FIStream * stream);
	~UClass();
};

struct FPushedState : FReadable {
	UState * state;
	UStruct * node;

	FPushedState();
	FPushedState(FIStream * stream);
};

struct FStateFrame : FReadable {
	UState * node;
	UState * stateNode;
	u_int64 probeMask;
	uint16 latentAction;
	FArray<FPushedState> stateStack;
	int32 offset;

	FStateFrame();
	FStateFrame(FIStream * stream);
};

#endif