#pragma once

#include "UE3.h"
#include "FIStream.h"
#include "Structure.h"

#include "DCAdaptors.h"

static INT					GSavingCompressionChunkSize = SAVING_COMPRESSION_CHUNK_SIZE;
static BOOL					GAlwaysBiasCompressionForSize = FALSE;

//NA x64 Patch:98.02 
//16577B197E1811C206FF76FF04C9011C
//7DCA909F20D01D512DC1A0522C346C39
#define DC_KEY "04C9011C06FF76FF7E1811C216577B19"
#define DC_IV "2C346C392DC1A05220D01D517DCA909F"

//Revision: 366226
//#define DC_KEY "B5AE92062ACC7452EB6FBB44EC2D612D"
//#define DC_IV "11E5B808F6922217F6FFB977321A0960"

struct DCTool : UIElement<UIElementType::DC> {
	bool Initialize();
	void RenderUI() override;

	void Reset() {

	}

	DCTool() {
		FileName[0] = 0;
		SaveFileName[0] = 0;

		//strcpy_s(FileName, "DC.bin");
		strcpy_s(FileName, "DC_366226.dat" /*"DataCenter_Final_TW.366226.unpacked"*/);
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

	char						Key[33] = DC_KEY;
	char						IV[33] = DC_KEY;

	S1DataCenter::S1DataCenter	DataCenter;
};
