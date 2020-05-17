#include "../App.h"

DWORD GCRCTable[256];

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
};

FCRCTableInit	GFCRCTableInit;

FBufferReader* FBufferReader::FromFile(const char* fileName)
{
	std::ifstream File;

	File.open(fileName);

	LARGE_INTEGER i;
	i.QuadPart = 0;
	//QueryPerformanceCounter(&i);

	bool Success = true;
	if (!File.is_open()) {
		Success = false;
		return nullptr;
	}

	File.seekg(0, std::fstream::end);
	auto Size = (size_t)File.tellg();
	File.seekg(0, std::fstream::beg);

	if (Size < 100) {
		File.close();
		Success = false;
		return nullptr;
	}

	auto buffer = std::unique_ptr<char[]>((char*)malloc(Size));
	auto* uBuffer = (char*)_aligned_malloc(Size, 8);

	File.read(buffer.get(), Size);

	File.close();

	auto FBuffer = std::make_unique<FBufferReader>((void*)buffer.get(), Size, FALSE);

	if (!appUncompressMemory(ECompressionFlags::COMPRESS_ZLIB, uBuffer, Size * 4, buffer.get(), Size)) {
		return nullptr;
	}

	if (FBuffer->ArForceByteSwapping) {
		INT j = 3;
		auto* Bit = ((char*)&i) + 3;
		do {

			FBuffer->SerializeBits((void*)&Bit, 1);
			Bit--;

			--j;
		} while (j >= 0);
	}
	else {
		FBuffer->SerializeBits((void*)&i, 4);
	}

	FBuffer->SerializeBits2((void*)&FBuffer->BitMap, 8);

	ULONG OutBits = 0;
	if (FBuffer->ArForceByteSwapping) {
		INT j = 3;
		auto* Bit = ((char*)&OutBits) + 3;
		do {

			FBuffer->SerializeBits((void*)&Bit, 1);
			Bit--;
			--j;
		} while (j >= 0);
	}
	else {
		FBuffer->SerializeBits((void*)&OutBits, 4);
	}

	if (i.LowPart == 6) {
		Message("Good version!");
	}


	return FBuffer.release();
}

void FBufferReader::SaveToFile(const char* fileName)
{
	std::ofstream file = std::ofstream(fileName);

	file.write((const char*)ReaderData, ReaderSize);

	file.close();
}
