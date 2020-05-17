#include "typeDefs.h"

int NormalizeAxis(int Angle)
{
	Angle &= 0xFFFF;
	if (Angle > 32767)
	{
		Angle -= 0x10000;
	}
	return Angle;
}

#include <string.h>

bool StartsWith(const char * a, const char * b)
{
	if (strncmp(a, b, strlen(b)) == 0) return 1;
	return 0;
}

int EndsWith(const char * str, const char * suffix)
{
	int32 str_len = strlen(str);
	int32 suffix_len = strlen(suffix);

	return
		(str_len >= suffix_len) &&
		(0 == strcmp(str + (str_len - suffix_len), suffix));
}

short Height_T(int x, int y, int maxX, int maxY, short * heights)
{
	x = MAX(MIN(x, maxX - 1), 0);
	y = MAX(MIN(y, maxY - 1), 0);
	return heights[y * maxY + x];
}

int32 DirectoryExists(const char * dirName_in) {
	DWORD ftyp = GetFileAttributesA(dirName_in);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return FALSE;

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return TRUE;

	return FALSE;
}

int32 FileExists(const char * dirName_in) {
	DWORD ftyp = GetFileAttributesA(dirName_in);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return FALSE;

	return TRUE;
}


