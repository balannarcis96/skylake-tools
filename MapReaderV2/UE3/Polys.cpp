#include "Polys.h"
#include "FIStream.h"
#include "FVector.h"

void Polys::ReadData(FIStream & data) 
{
	UObject::ReadData(data);

	//data._pos = rawDataOffset;
	//dbNum = data.ReadInt32();
	//dbMax = data.ReadInt32();
	//owner = UObject::ReadFromStream(&data);

	//for (int32 i = 0; i < dbNum; i++)
	//{
	//	FVector3 v = FVector3(&data);
	//	v = FVector3(&data);
	//	v = FVector3(&data);
	//	v = FVector3(&data);
	//}
}
