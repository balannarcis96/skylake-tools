#include "../App.h"

constexpr auto MAX_DC_FILE_SIZE = (1024 * 1024 * 1024);

#include "AES.h"
//#include "AES2.h"

//#include <AES256.hpp>

//@TODO use faster lib
bool DecryptDC(FIStream& Stream, char Key[16], char IV[16]) {

	//Varaint 1
	AES aes(128);

	auto* DecryptedDc = aes.DecryptCFB((unsigned char*)Stream._raw, (int)Stream._size, (unsigned char*)Key, (unsigned char*)IV);
	if (!DecryptedDc) {
		return false;
	}

	memcpy_s(Stream._raw, Stream._size, DecryptedDc, Stream._size);

	free(DecryptedDc);

	return true;
}

bool EncryptDC(FIStream& Stream, char Key[16], char IV[16]) {

	AES aes(128);

	UINT NewLen = 0;
	auto* EncryptedDc = aes.EncryptCFB((unsigned char*)Stream._raw, (int)Stream._size, (unsigned char*)Key, (unsigned char*)IV, NewLen);
	if (!EncryptedDc) {
		return false;
	}

	if (NewLen < Stream._size) {
		Message("Different size after encryption before %d after %d", Stream._size, NewLen);
	}

	//Stream.Clear();
	//Stream.Resize(NewLen);

	Stream.SetFront();
	Stream.Write(EncryptedDc, Stream._size);

	free(EncryptedDc);

	return true;
}

bool UncompressDC(FIStream& Stream) {

	UINT32 UncompressedSize = Stream.ReadUInt32();
	if (UncompressedSize >= INT_MAX) {
		return false;
	}

	auto buffer = std::unique_ptr<char[]>(new char[UncompressedSize]);

	if (!appUncompressMemoryZLIB(buffer.get(), UncompressedSize, (const void*)(Stream._raw + Stream._pos), Stream._size - Stream._pos)) {
		return false;
	}

	Stream.Clear();
	Stream.Write((UINT8*)buffer.get(), UncompressedSize);
	Stream._pos = 0;

	return true;
}

bool CompressDC(FIStream& Stream) {

	UINT UncompressedSize = Stream._size;
	INT CompressedSize = (INT)Stream._size;
	auto buffer = std::unique_ptr<char[]>(new char[CompressedSize]);

	if (!appCompressMemoryZLIB(buffer.get(), CompressedSize, (const void*)(Stream._raw), Stream._size)) {
		return false;
	}

	Stream.Clear();
	Stream.Resize(CompressedSize + 4);
	Stream.WriteUInt32(UncompressedSize);
	Stream.Write((BYTE*)buffer.get(), CompressedSize);

	return true;
}

bool DCTool::Initialize()
{
	if (!Base::Initialize()) {
		return false;
	}

	Show();

	return true;
}

void DCTool::RenderUI()
{
#if DC_64
	ImGui::Begin("DC Tool (For x64 DataCenters Only)", &bDisplay);
#else
	ImGui::Begin("DC Tool (For x32 DataCenters Only)", &bDisplay);
#endif

	if (ImGui::BeginTabBar("Tabs"))
	{
		if (ImGui::BeginTabItem("Main"))
		{
			ImGui::TextColored(INFO_COLOR, "Key");
			ImGui::InputText("<", (char*)Key, 33);
			ImGui::TextColored(INFO_COLOR, "IV");
			ImGui::InputText("<<", (char*)IV, 33);

			ImGui::Separator();
			ImGui::TextColored(INFO_COLOR, "## Load ##");
			ImGui::Checkbox("IsEncrypted", &IsEncrypted);
			ImGui::SameLine();
			ImGui::Checkbox("IsCompressed", &IsCompressed);
			ImGui::InputText("DC FileName", FileName, 1024);

			if (ImGui::Button("Load DC")) {
				do {
					if (strnlen_s(FileName, 1024) == 0) {
						Message("Please specify the FileName");
						break;
					}

					auto DCFileName = GD3DDriver.BuildDCPath(FileName);

					if (!LoadDC(DCFileName.c_str(), IsEncrypted, IsCompressed)) {
						Message("Failed to load the DC");
						break;
					}

					Message("DC loaded successfully");

				} while (0);
			}

			ImGui::Separator();
			ImGui::TextColored(INFO_COLOR, "## Save ##");
			ImGui::Checkbox("Encrypt", &Encrypt);
			ImGui::SameLine();
			ImGui::Checkbox("Compress", &Compress);
			ImGui::InputText("Save DC FileName", SaveFileName, 1024);

			if (ImGui::Button("Save DC")) {
				do {
					if (strnlen_s(SaveFileName, 1024) == 0) {
						Message("Please specify the SaveFileName");
						break;
					}

					auto DCFileSaveName = GD3DDriver.BuildDCPath(SaveFileName);

					if (!SaveDC(DCFileSaveName.c_str(), Encrypt, Compress)) {
						Message("Failed to save the DC");
						break;
					}

					Message("DC saved successfully");

				} while (0);
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Edit"))
		{
			ImGui::Text("Edit...");

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}

bool DCTool::LoadDC(const char* FileName, bool IsEncrypted, bool IsCompressed)
{
	FIStream Stream;
	if (!Stream.load_from_file(FileName)) {
		Message("Failed to read file [%s]", FileName);
		return false;
	}

	if (IsEncrypted) {
		uint8_t Key[16];
		uint8_t IV[16];

		GetKey(Key);
		GetIV(IV);

		if (!DecryptDC(Stream, (char*)Key, (char*)IV)) {
			Message("Failed to decrypt the DC");
			return false;
		}
	}

	if (IsCompressed) {
		if (!UncompressDC(Stream)) {
			Message("Failed to uncompress the DC");
			return false;
		}
	}

	//Set loading stream
	Stream._isLoading = true;

	if (!SerializeDC(Stream)) {
		Message("Failed to parse the DC");
		return false;
	}

	if (!DataCenter.Prepare()) {
		return false;
	}

	return true;
}

bool DCTool::SaveDC(const char* FileName, bool Encrypt, bool Compress)
{
	FIStream SaveStream;
	SaveStream.Resize(MAX_DC_FILE_SIZE);
	SaveStream._isLoading = false;

	if (!SerializeDC(SaveStream)) {
		Message("Failed to save the DC");
		return false;
	}

	FIStream Temp;
	Temp.Resize(SaveStream._pos);
	Temp.Write(SaveStream._raw, SaveStream._pos);

	SaveStream.Clear();
	SaveStream.Acquire(Temp);

	//Message("DC Serialized");

	if (Compress) {
		if (!CompressDC(SaveStream)) {
			Message("Failed to compress DC");
			return false;
		}

		//Message("DC Compressed");
	}

	if (Encrypt) {
		uint8_t Key[16];
		uint8_t IV[16];

		GetKey(Key);
		GetIV(IV);

		if (!EncryptDC(SaveStream, (char*)Key, (char*)IV)) {
			Message("Failed to encrypt DC");
			return false;
		}

		Message("DC Encrypted");
	}

	SaveStream.save_to_file(FileName);

	Message("DC Saved size=%d", SaveStream._size);

	return true;
}

bool DCTool::SerializeDC(FIStream& Stream)
{
	if (!DataCenter.Serialize(Stream)) {
		return false;
	}

	return true;
}

//Globals
DCTool				GDCTool;