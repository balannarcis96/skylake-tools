#ifndef FROTATOR_H
#define FROTATOR_H

#include "FReadable.h"
#include "UMath.h"

struct FIStream;

struct  FRotator :FReadable{

	int
		pitch,
		yaw,
		roll;

	FRotator operator+(const FRotator& R) const{
		return FRotator(pitch + R.pitch, yaw + R.yaw, roll + R.roll);
	}
	FRotator operator-(const FRotator& R) const{
		return FRotator(pitch - R.pitch, yaw - R.yaw, roll - R.roll);
	}
	FRotator operator*(FLOAT Scale) const{
		return FRotator(appTruncL(pitch * Scale), appTruncL(yaw * Scale), appTruncL(roll * Scale));
	}
	FRotator operator*= (FLOAT Scale){
		pitch = appTruncL(pitch * Scale); yaw = appTruncL(yaw * Scale); roll = appTruncL(roll * Scale);
		return *this;
	}
	void operator= (const FRotator& other) {
		pitch = other.pitch;
		yaw = other.yaw;
		roll = other.roll;
	}

	FRotator(){ pitch = 0; yaw = 0; roll = 0; }
	FRotator(const FRotator& other){ pitch = other.pitch; yaw = other.yaw; roll = other.roll; }
	FRotator(FIStream* stream);
	FRotator(int p, int y, int r);

	const Vector3 euler();
	void  normalize(int components[3]);
	void  denormalize(int components[3]);
};


#endif