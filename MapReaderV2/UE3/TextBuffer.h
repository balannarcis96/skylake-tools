#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include "FReadable.h"
#include "FString.h"
#include "UObject.h"

struct TextBuffer : UObject {
	int32 pos;
	int32 top;
	FString text;

	TextBuffer();

protected:
	void ReadData(FIStream & data);
};

#endif