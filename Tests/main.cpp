#include <iostream>

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef wchar_t				UNICHAR;	// A unicode character. normally a signed type.
typedef wchar_t				TCHAR;	// A unicode character. normally a signed type.

#define __TEXT(quote) L##quote      // r_winnt
#define TEXT(quote) __TEXT(quote)   // r_winnt

DWORD GCRCTable[256];
#define CRC32_POLY 0x04c11db7
class FCRCTableInit
{
public:
	/**
	* Constructor
	*/
	FCRCTableInit()
	{
		// Init CRC table.
		for (DWORD iCRC = 0; iCRC < 256; iCRC++)
		{
			for (DWORD c = iCRC << 24, j = 8; j != 0; j--)
			{
				GCRCTable[iCRC] = c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
			}
		}
	}

	static FCRCTableInit Instance;
};

FCRCTableInit GFCRCTableInit;

#define UPPER_LOWER_DIFF	32
inline TCHAR appToUpper(TCHAR c)
{
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	//@hack - ideally, this would be data driven or use some sort of lookup table
	// some special cases
	switch (UNICHAR(c))
	{
		// these special chars are not 32 apart
	case 255: return 159; // diaeresis y
	case 156: return 140; // digraph ae

	// characters within the 192 - 255 range which have no uppercase/lowercase equivalents
	case 240:
	case 208:
	case 223:
	case 247:
		return c;
	}

	if ((c >= TEXT('a') && c <= TEXT('z')) || (c > 223 && c < 255))
	{
		return c - UPPER_LOWER_DIFF;
	}

	// no uppercase equivalent
	return c;
}

/** Case insensitive string hash function. */
inline DWORD appStrihash(const TCHAR* Data)
{
	DWORD Hash = 0;
	while (*Data)
	{
		TCHAR Ch = appToUpper(*Data++);
		WORD  B = Ch;
		Hash = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
		B = Ch >> 8;
		Hash = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
	}
	return Hash;
}

/** Case sensitive string hash function. */
inline DWORD appStrihashEx(const TCHAR* Data)
{
	DWORD Hash = 0;
	while (*Data)
	{
		TCHAR Ch = *Data++;
		WORD  B = Ch;
		Hash = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
		B = Ch >> 8;
		Hash = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
	}
	return Hash;
}

void main() {
	auto hash = appStrihash(L"String");

	return;
}