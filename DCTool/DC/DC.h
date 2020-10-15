#pragma once

#include "UE3.h"
#include "FIStream.h"
#include "Structure.h"

#include "DCAdaptors.h"

static INT					GSavingCompressionChunkSize = SAVING_COMPRESSION_CHUNK_SIZE;
static BOOL					GAlwaysBiasCompressionForSize = FALSE;

#define EU_9303_KEY "457D8D7F84C0D759FB8C802FBDEC3264"
#define EU_9303_IV "5ADB85308FFD0A33B9B7D32F0AE42255"

//Revision: 366226
//#define EU_9303_KEY "B5AE92062ACC7452EB6FBB44EC2D612D"
//#define EU_9303_IV "11E5B808F6922217F6FFB977321A0960"

struct DCTool : UIElement<UIElementType::DC> {
	bool Initialize();
	void RenderUI() override;

	void Reset() {

	}

	DCTool() {
		FileName[0] = 0;
		SaveFileName[0] = 0;

		//strcpy_s(FileName, "DC.bin");
		strcpy_s(FileName, "DataCenter_Final_TW.366226.unpacked");
		strcpy_s(SaveFileName, "Saved_32.bin");
		strcpy_s(ImportBasePath, "C:/DC_366226/");
		wcscpy_s(ExportBasePath, L"C:/");
	}

	void GetKey(uint8_t OutKey[16])const noexcept {
		appHexDecode(Key, 32, OutKey);
	}
	void GetIV(uint8_t OutIV[16])const noexcept {
		appHexDecode(IV, 32, OutIV);
	}

private:
	bool LoadDC(const char* FileName, bool IsEncrypted = true, bool IsCompressed = true);
	bool SaveDC(const char* FileName, bool Encrypt = true, bool Compress = true);
	bool SerializeDC(FIStream& Stream);

	bool						IsEncrypted = false;
	bool						IsCompressed = false;
	char						FileName[1024];

	bool						Encrypt = false;
	bool						Compress = false;
	char						SaveFileName[1024];
	wchar_t						ExportBasePath[MAX_PATH];
	char						ImportBasePath[MAX_PATH];
	
	INT							DCFilesType = 0;

	char						Key[33] = EU_9303_KEY;
	char						IV[33] = EU_9303_IV;

	S1DataCenter::S1DataCenter	DataCenter;
};
