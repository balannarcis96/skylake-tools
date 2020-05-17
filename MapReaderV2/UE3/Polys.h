#ifndef POLYS_H
#define POLYS_H

#include "UObject.h"

struct Polys : UObject {
	int32 dbNum;
	int32 dbMax;
	UObject * owner;
protected:
	void ReadData(FIStream& data);
};

#endif