#include "UClass.h"

UStruct::UStruct(FIStream * stream)
	:UField(stream)
{
	scriptText = (TextBuffer*)UObject::ReadFromStream(stream);
	children = (UField*)UObject::ReadFromStream(stream);
	cppText = (TextBuffer*)UObject::ReadFromStream(stream);
	line = stream->ReadInt32();
	textPos = stream->ReadInt32();

	int32 scriptBytecodeSize = 0;
	scriptBytecodeSize = stream->ReadInt32();
	if (scriptBytecodeSize) {
		scriptData = new uint8[scriptBytecodeSize];
		stream->Read(scriptData, scriptBytecodeSize);

		//TODO: decompile script add show it in the editor
	}
}

UStruct::~UStruct()
{
	if (scriptData) {
		delete[] scriptData;
		scriptData = nullptr;
	}
}

const bool UField::IsNative() const
{
	return true;
}

UField::UField(FIStream * stream)
	:UObject(stream)
{
	superField = (UField*)UObject::ReadFromStream(stream);
	next = (UField*)UObject::ReadFromStream(stream);
}

FImplementedInterface::FImplementedInterface()
{
	objectClass = pointerProperty = nullptr;
}

FImplementedInterface::FImplementedInterface(FIStream * stream)
	:FReadable(stream)
{
	objectClass = UObject::ReadFromStream(stream);
	pointerProperty = UObject::ReadFromStream(stream);
}

UState::UState(FIStream * stream)
	:UStruct(stream)
{
	unk = stream->ReadInt32();
	probeMask = stream->ReadInt32();
	ignoreMask = stream->ReadInt64();
	labelTableOffset = stream->ReadInt16();
	stateFlags = stream->ReadInt32();
	funcMap = new FMap<FName*, UObject *>(stream);
}

UState::~UState()
{
	if (funcMap) {
		delete funcMap;
		funcMap = nullptr;
	}
}

UClass::UClass(FIStream * stream)
	:UState(stream)
{
	classFlags = stream->ReadInt32();
	classWithin = (UClass*)UObject::ReadFromStream(stream);
	classConfigName = FName(stream);
	hideCategories = FArray<FName>(stream);
	componentNameToDefaultObjectMap = new FMap<FName*, UObject*>(stream);
	interferences = FArray<FImplementedInterface>(stream);
	autoExpandedCategories = FArray<FName>(stream);
	classDefaultObject = UObject::ReadFromStream(stream);
}

UClass::~UClass()
{
	if (componentNameToDefaultObjectMap) {
		delete componentNameToDefaultObjectMap;
		componentNameToDefaultObjectMap = nullptr;
	}
}

FStateFrame::FStateFrame()
{
}

FStateFrame::FStateFrame(FIStream * stream)
	:FReadable(stream)
{
	node = (UState*)UObject::ReadFromStream(stream);
	stateNode = (UState*)UObject::ReadFromStream(stream);
	probeMask = stream->ReadInt64();
	latentAction = stream->ReadInt16();
	stateStack = FArray<FPushedState>(stream);
	if (node && !node->IsZero()) {
		offset = stream->ReadInt32();
	}
}

FPushedState::FPushedState()
{
	state = nullptr;
	node= nullptr;
}

FPushedState::FPushedState(FIStream * stream)
	:FReadable(stream)
{
	state = (UState*)UObject::ReadFromStream(stream);
	node = (UStruct*)UObject::ReadFromStream(stream);
}
