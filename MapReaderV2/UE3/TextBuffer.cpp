#include "TextBuffer.h"

TextBuffer::TextBuffer()
	:UObject()
{
}

void TextBuffer::ReadData(FIStream & data)
{
	data._pos = rawDataOffset;
	pos = data.ReadInt32();
	top = data.ReadInt32();
	text = FString(&data);
}
