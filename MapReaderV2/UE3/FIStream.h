////////////////////////////////////////////////////////////
/////////////////Writed by Balan Narcis/////////////////////
////////Copyright © Balan Narcis.All rights reserved.///////
////////////////////////////////////////////////////////////

//use only for big-edian

#ifndef FILESTREAM_H
#define FILESTREAM_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include "typeDefs.h"

struct UPackage;

struct FIStream
{
	friend bool decompressPackage(UPackage*);

	FIStream();
	FIStream(u_int64 size);
	FIStream(uint8* data, u_int64 size);
	FIStream(std::istream * str);

	FIStream(FIStream&);
	~FIStream();

	bool				load_from_file(const char*);
	void				save_to_file(const char*);


	//WRITE_*******************************************************
	void				Resize(u_int64 size);

	void				Write(uint8* data, u_int64 size);

	void				WriteString(std::string s, bool L_ED = false);

	void				WriteUInt8(uint8);

	void				WriteInt16(int16);
	void				WriteUInt16(uint16);

	void				WriteInt32(int32);
	void				WriteUInt32(int32);

	void				Write_int64(_int64);
	void				WriteU_int64(u_int64);

	void				WriteFloat(float);
	void				WriteDouble(double);

	void				WritePos(uint16, int16 offset = 0);



	//READ_********************************************************
	void				Read(uint8* out_buffer, u_int64 size);

	char				ReadInt8();
	uint8				ReadUInt8();

	int16				ReadInt16();
	uint16				ReadUInt16();

	int32				ReadInt32();
	uint32				ReadUInt32();

	_int64				ReadInt64();
	u_int64				ReadUInt64();

	float				ReadFloat();
	double				ReadDouble();

	std::string			ReadUTF16StringLittleEdianToASCII();
	std::string			ReadUTF16StringBigEdianToASCII();
	std::string			ReadASCIIString();
	bool				ReadASCIIStringTo(uint8 * out, uint16 max_len);

	
	
	//MISC_**************************************************
	void				Clear();
	void				Zero();
	u_int64				SetEnd();
	u_int64				SetFront();
	u_int64				NextPos();

	u_int64 NextPos32()
	{
		uint16 tempPos = _pos;
		WriteUInt32(0);
		return tempPos;
	}
	uint8*				GetPtr();

	//MEMBER_VAR_**************************************************
	uint8*				_raw;
	u_int64				_size;
	u_int64				_pos;

	UPackage  *			_package;
};

#endif



