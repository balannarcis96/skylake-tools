#include "Texture2D.h"
#include "FPropertyTag.h"

void Texture2D::ReadData(FIStream & data)
{
	UObject::ReadData(data);

	data._pos = rawDataOffset;

	sourceArt = FByteBulkData(&data);
	source = FString(&data);
	mips = FArray<FMipMap>(&data);
	guid = FGUID(&data);
}

const char * Texture2D::TextureCompression() const
{
	FPropertyTag * prop = GetPropByName("CompressionSettings");
	if (!prop || !prop->value) {
		return nullptr;
	}

	return ((FString*)prop->value)->string.c_str();
}

