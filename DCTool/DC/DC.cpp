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
			ImGui::TextColored(SUCCESS_COLOR, "DCToolState: %s", DataCenter.GetDCToolStateStr());
			if (DataCenter.IsLoaded()) {
				ImGui::SameLine();
				ImGui::TextColored(SUCCESS_COLOR, "| Format Version: %d", DataCenter.FormatVersion);
				ImGui::SameLine();
				ImGui::TextColored(SUCCESS_COLOR, "| Version: %d", DataCenter.Version);
			}
			ImGui::Separator();
			ImGui::TextColored(INFO_COLOR, "## Load ##");
			ImGui::Checkbox("IsEncrypted", &IsEncrypted);
			ImGui::SameLine();
			ImGui::Checkbox("IsCompressed", &IsCompressed);
			ImGui::InputText("DC FileName", FileName, 1024);

			if (!DataCenter.IsLoaded()) {
				ImGui::NewLine();
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
			}

			if (!DataCenter.IsLoaded()) {
				ImGui::Separator();
				ImGui::TextColored(INFO_COLOR, "## From DC Files ##");
				ImGui::InputText("DC Files Dir", ImportBasePath, MAX_PATH);

				ImGui::RadioButton("XML", &DCFilesType, 0);
				ImGui::SameLine();
				ImGui::RadioButton("JSON", &DCFilesType, 1);

				static bool CompreseDC = false;
				static bool EncryptDC = false;
				if (ImGui::Checkbox("Compress", &CompreseDC)) {
					if (CompreseDC) {
						EncryptDC = false;
					}
				}

				if (CompreseDC) {
					ImGui::SameLine();
					ImGui::Checkbox("Encrypt", &EncryptDC);
				}
			
				ImGui::NewLine();
				if (ImGui::Button("Build DC")) {
					DataCenter.Clear();

					do {
						wchar_t DirName[MAX_PATH];

						INT Len = 0;
						Len = MultiByteToWideChar(0, 0, ImportBasePath, strnlen_s(ImportBasePath, MAX_PATH), DirName, MAX_PATH);
						if (!Len) {
							Message("An unexpected error occurred!");
							break;
						}

						DirName[Len] = L'\0';

						if (DCFilesType == 0) {
							DCAdaptors::DCXMLAdaptor xmlAddaptor;

							if (!xmlAddaptor.BuildDC(DirName, &DataCenter)) {
								Message("Failed to build the DC from XML files!");
								break;
							}
						}
						else if (DCFilesType == 1) {
							DCAdaptors::DCJsonAdaptor jsonAddaptor;

							if (!jsonAddaptor.BuildDC(DirName, &DataCenter)) {
								Message("Failed to build the DC from JSON files!");
								break;
							}
						}
						else {
							Message("Unknown DC files format!");
							break;
						}

						FIStream DCStream;
						DCStream._isLoading = false;
						DCStream.Resize(1024 * 1024 * 1024); //1gb

						if (!DataCenter.Serialize(DCStream)) {
							Message("Failed to serialize DC!");
							break;
						}

						if (CompreseDC) {
							if (!CompressDC(DCStream)) {
								Message("Failed to compress DC");
								break;
							}

							if (EncryptDC) {
								if (!::EncryptDC(DCStream, Key, IV)) {
									Message("Failed to encrypt DC");
									break;
								}
							}
						}

						//@TODO compress and encrypt here

						std::string DCFileName = ImportBasePath;
						DCFileName += "/DC_";
						DCFileName += std::to_string(DataCenter.Version);
						DCFileName += ".bin";

						if (!DCStream.save_to_file(DCFileName.c_str())) {
							Message("Failed to save DC file [%s]!", DCFileName.c_str());
							break;
						}

						Message("Successfully Build DC file[%s]", DCFileName.c_str());

					} while (0);
				}
			}

			if (DataCenter.IsLoaded()) {
				if (ImGui::Button("Clear DC")) {
					DataCenter.Clear();
				}
			}

			if (DataCenter.IsLoaded()) {
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

				ImGui::Separator();
				//ImGui::InputText("Export Path", ExportBasePath, MAX_PATH);
				if (ImGui::Button("Export to XML"))
				{
					DCAdaptors::DCXMLAdaptor adaptor;

					do {
						if (!adaptor.FromDC(&DataCenter)) {
							Message("Failed to build XML adaptor");
						}

						if (!adaptor.Export(ExportBasePath)) {
							Message("Failed to export XML data");
						}
					} while (0);
				}
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
	DataCenter.Clear();

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
