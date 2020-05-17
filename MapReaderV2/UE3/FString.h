#ifndef FSTRING_H
#define FSTRING_H

#include "FReadable.h"
#include "FArray.h"

#include "typeDefs.h"

#include <string.h>
#include "Enums.h"

struct FIStream;
struct FStringArray;

struct FString :FReadable {

	std::string string;
	int32 length;
	int32 bIsUnicode;
	uint32 flags;

	FString() {}
	FString(FIStream * stream);
	virtual  ~FString();

	uint32 StrLen() const {
		return string.length();
	}

	bool IsEqualWith(const char* str) {
		return strcmp(str, string.c_str()) == 0;
	}
};

struct FNamePair :FString {
	EObjectFlags rfObjectFlags;

	FNamePair();
	FNamePair(FIStream *);
};

struct FUrl :FReadable {
	FString
		protocol,
		host,
		map,
		portal;

	int32 opCount;
	FString*  op;

	int32 port;
	int32 bValid;

	FUrl();
	FUrl(FIStream * stream);
	~FUrl();
};

struct FName :FReadable {
	int32 nameIndex;
	int32 index;

	FName();
	FName(FIStream * stream);

	const char* operator()() const;

	const char * Name() const;

	int32 NameLength() const;

	const bool IsEqual(const char* str) const {
		return strE((*this)(), str);
	}
};

#endif
