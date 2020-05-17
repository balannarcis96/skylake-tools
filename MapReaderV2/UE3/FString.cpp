#include "FString.h"
#include "FIStream.h"
#include "UPackage.h"

FString::FString(FIStream * stream)
	:FReadable(stream)
{
	length = stream->ReadInt32();

	//if length is negative -> UTF16 encoding , else UTF8

	if (length == 0) {
		bIsUnicode = 0;
		flags = 0;
	}
	else if (length > 0) {
		char * str = new char[length];
		stream->Read((uint8*)str, length);
		string = std::string(str);
		bIsUnicode = 0;

		delete[] str;
	}
	else {
		length = -length;
		wchar_t * wstr = new wchar_t[length];
		stream->Read((uint8*)wstr, length * 2);
		std::wstring _wstr = std::wstring(wstr);
		string = std::string(_wstr.begin(), _wstr.end());

		_wstr = L"";

		delete[] wstr;
		bIsUnicode = 1;
	}
}

FString::~FString()
{

}

FName::FName()
{
}

FName::FName(FIStream * stream)
	:FReadable(stream)
{
	nameIndex = stream->ReadInt32();
	index = stream->ReadInt32();
}

const char * FName::operator()() const
{
	return package->nameForIndex(nameIndex);
}

const char * FName::Name() const
{
	return package->nameForIndex(nameIndex);
}

int32 FName::NameLength() const
{
	return strlen(package->nameForIndex(nameIndex));
}

FUrl::FUrl()
{
	op = nullptr;
}

FUrl::FUrl(FIStream * stream)
	:FReadable(stream) {

	protocol = FString(stream);
	host = FString(stream);
	map = FString(stream);
	portal = FString(stream);

	opCount = stream->ReadInt32();
	if (opCount) {
		op = new FString[opCount];
		for (int32 i = 0; i < opCount; i++)
		{
			op[i] = FString(stream);
		}
	}
	else {
		op = nullptr;
	}

	port = stream->ReadInt32();
	bValid = stream->ReadInt32();
}

FUrl::~FUrl()
{
	if (op) {
		delete[] op;
		op = nullptr;
	}
}

FNamePair::FNamePair()
	:FString()
{}

FNamePair::FNamePair(FIStream * stream)
	: FString(stream)
{
	rfObjectFlags = (EObjectFlags)stream->ReadInt64();
}
