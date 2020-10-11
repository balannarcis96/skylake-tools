#include "../App.h"

#include <filesystem>

#define _CRT_NON_CONFORMING_SWPRINTFS 1

#include <direct.h>
#include <stdio.h>
#include <unordered_map>
#include <string_view>

std::unordered_map<std::wstring, WORD> AttributeTypes;

wchar_t* PrintElementName(wchar_t* Buffer, S1DataCenter::ElementItem* Element, INT Depth) noexcept {
	wchar_t* Cursor = Buffer;

	for (INT i = 0; i < Depth; i++)
	{
		*(Cursor++) = L' ';
		*(Cursor++) = L' ';
	}

	*(Cursor++) = L'<';

	const auto NameLen = wcslen(Element->NameRef.Get());
	memcpy_s(Cursor, NameLen * sizeof(wchar_t), Element->NameRef.Get(), NameLen * sizeof(wchar_t));
	Cursor += NameLen;
	*Cursor = L' ';

	return ++Cursor;
}

wchar_t* PrintAttributes(wchar_t* Buffer, S1DataCenter::ElementItem* Element)noexcept {
	if (!Element->Attributes.size()) {
		goto print_element_end;
	}

	for (auto* Attribute : Element->Attributes) {
		if (!Attribute->CacheValue()) {
			Message("failed to cache value for attribute [%ws] on element[%ws]", Attribute->NameRef.Get(), Element->NameRef.Get());
			continue;
		}

		size_t Len = wcslen(Attribute->NameRef.Get());
		memcpy_s(Buffer, Len * sizeof(wchar_t), Attribute->NameRef.Get(), Len * sizeof(wchar_t));
		Buffer += Len;

		std::wstring AttrFullName = Element->NameRef.Get();
		AttrFullName += L'.';
		AttrFullName += Attribute->NameRef.Get();

		auto Item = AttributeTypes.find(AttrFullName);
		if (Item == AttributeTypes.end()) {
			AttributeTypes.insert(std::pair<std::wstring, WORD>(AttrFullName, (WORD)Attribute->TypeInfo));
		}

		*(Buffer++) = L'=';
		*(Buffer++) = L'\"';

		Len = wcslen(Attribute->StringyfiedValue.c_str());
		memcpy_s(Buffer, Len * sizeof(wchar_t), Attribute->StringyfiedValue.c_str(), Len * sizeof(wchar_t));
		Buffer += Len;

		*(Buffer++) = L'\"';
		*(Buffer++) = L' ';
	}

print_element_end:
	if (!Element->Children.size()) {
		*(Buffer++) = L'/';
	}

	*(Buffer++) = L'>';
	*(Buffer++) = L'\n';

	return Buffer;
}

wchar_t* XmlFromDCElement(TRef<S1DataCenter::ElementItem> Element, wchar_t* Buffer, INT Depth = 0)noexcept {

	if (Element->Name == 0) {
		return Buffer;
	}

	Buffer = PrintElementName(Buffer, Element.Get(), Depth);
	Buffer = PrintAttributes(Buffer, Element.Get());

	for (auto& RawChild : Element->Children) {
		Buffer = XmlFromDCElement(RawChild, Buffer, Depth + 1);
	}

	if (Element->Children.size()) {
		for (INT i = 0; i < Depth; i++)
		{
			*(Buffer++) = L' ';
			*(Buffer++) = L' ';
		}

		*(Buffer++) = L'<';
		*(Buffer++) = L'/';
		size_t Len = wcslen(Element->NameRef.Get());
		memcpy_s(Buffer, Len * sizeof(wchar_t), Element->NameRef.Get(), Len * sizeof(wchar_t));
		Buffer += Len;
		*(Buffer++) = L'>';
		*(Buffer++) = L'\n';
	}

	return Buffer;
}

bool DCAdaptors::DCJsonAdaptor::FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter)
{
	return false;
}

bool DCAdaptors::DCJsonAdaptor::Export(const wchar_t* FileName)
{
	return true;
}

bool DCAdaptors::DCXMLAdaptor::FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter)
{
	this->DataCenter = DataCenter;
	auto RootElement = DataCenter->GetRootElement();

	return true;
}

bool DCAdaptors::DCXMLAdaptor::Export(const wchar_t* RootDir)
{
	TRef<S1DataCenter::ElementItem> RootElement = DataCenter->GetRootElement();
	if (wcscmp(RootElement->NameRef.Get(), L"__root__")) {
		Message("Failed to find __root__ element");
		return false;
	}
	else {
		Message("Found __root__ (%ws) element, childrenCount: %lld\nClick OK to start exporting.", RootElement->NameRef.Get(), RootElement->Children.size());
	}

	wchar_t* buffer = new wchar_t[(1024 * 1024) * 512];

	std::unordered_map<std::wstring, INT> FilesMap;

	std::wstring BaseFilePath = RootDir;
	DataCenter->BuildDCExportName(BaseFilePath);
	_wmkdir(BaseFilePath.c_str());
	std::wstring currentSubDir;

	INT ValidCount = 0;

	for (auto* Element : RootElement->Children) {

		if (!Element->NameRef.Get()) {
			continue;
		}

		ValidCount++;

		INT FileIndex = 0;
		auto item = FilesMap.find(std::wstring(Element->NameRef.Get()));
		if (item == FilesMap.end()) {
			FilesMap.insert(std::pair<std::wstring, INT>(Element->NameRef.Get(), FileIndex));

			currentSubDir = Element->NameRef.Get();
			_wmkdir((BaseFilePath + L"/" + currentSubDir).c_str());
		}
		else {
			FileIndex = ++item->second;
		}

		std::wstring FileSavePath;
		FileSavePath += BaseFilePath;
		FileSavePath += L"/";
		FileSavePath += currentSubDir;
		FileSavePath += L"/";
		FileSavePath += Element->NameRef.Get();
		FileSavePath += L"-";
		FileSavePath += std::to_wstring(FileIndex);
		FileSavePath += L".xml";

		buffer[0] = '\0';
		wchar_t* end = XmlFromDCElement(Element, buffer);
		if (end == buffer) {
			continue;
		}

		*end = L'\0';

		std::ofstream outputFile = std::ofstream(FileSavePath.c_str(), std::ofstream::binary);

		outputFile.write((const char*)buffer, (end - buffer) * sizeof(wchar_t));

		outputFile.close();
	}

	delete[] buffer;

	if (!SerializeMetadata(RootDir, ValidCount)) {
		Message("Failed to save metadata");
		return false;
	}

	Message("DC exported to XML successfully");

	return true;
}

bool DCAdaptors::DCXMLAdaptor::BuildDC(const wchar_t* DirName, TRef<S1DataCenter::S1DataCenter> DataCenter)
{
	S1DataCenter::StateGuard<S1DataCenter::DCState::Building>(DataCenter.Get());

	this->DataCenter = DataCenter;

	if (!DataCenter->PrepareForBuild()) {
		Message("Failed to prepare string buckets, no memory T_T");
		return false;
	}

	std::wstring WDirName = DirName;
	std::string	 ADirName = std::string(WDirName.begin(), WDirName.end());

	std::wstring MetadataFile = WDirName;
	MetadataFile += L"DCMetadata.bin";

	INT FilesCount = 0;
	if (!ReadMetadata(MetadataFile.c_str(), FilesCount)) {
		return false;
	}

	std::vector<std::string> FileNames;
	if (!FindAllFilesInDirectory(ADirName.c_str(), ".xml", FileNames)) {
		Message("Failed to find .xml files in directory [%ws]", DirName);
		return false;
	}

	if (FileNames.size() != (size_t)FilesCount) {
		Message("Found xml files count mismatch, if not intended please revise.\n\nExpected [%lld] Found[%lld]", (size_t)FilesCount, FileNames.size());
	}

	DataCenter->Elements.

	constexpr size_t ReadBufferSize = 1024 * 1024 * 512;
	auto Buffer = std::unique_ptr<wchar_t>(new wchar_t[ReadBufferSize + 1]);

	for (size_t Index = 0; Index < FileNames.size(); Index++)
	{
		std::ifstream File = std::ifstream(FileNames[Index].c_str());
		if (!File.is_open()) {
			Message("Failed to open xml file[%s]", FileNames[Index].c_str());
			return false;
		}

		File.seekg(0, std::ifstream::end);
		size_t StreamSize = File.tellg();
		File.seekg(0, std::ifstream::beg);

		if (StreamSize == 0) {
			Message("File[%s] is empty", FileNames[Index].c_str());
			File.close();
			continue;
		}

		if (StreamSize % 2 != 0) {
			Message("File[%s] encoding mismatch, expected UTF16", FileNames[Index].c_str());
			File.close();
			continue;
		}

		if (StreamSize >= ReadBufferSize) {
			Message("File[%s] exceeds read buffer size[%lld]", ReadBufferSize);
			File.close();
			return false;
		}

		File.read((char*)Buffer.get(), StreamSize);
		File.close();

		Buffer.get()[StreamSize] = L'\0';

		if (!ParseXmlFile(Buffer.get(), StreamSize / 2)) {
			Message("Failed to parse xml file[%s]", FileNames[Index].c_str());
			return false;
		}
	}

	return true;
}

bool DCAdaptors::DCXMLAdaptor::ParseXmlFile(const wchar_t* Buffer, size_t BufferLength) noexcept
{
	return false;
}

bool DCAdaptors::DCAdaptor::SerializeMetadata(const wchar_t* RootDir, INT FilesCount) noexcept
{
	FIStream Stream;
	Stream._isLoading = false;
	Stream.Resize(2048);

	Stream.WriteInt32(FilesCount);

	Stream.WriteUInt32(DataCenter->FormatVersion);
	Stream.Write_int64(DataCenter->Unk1_8);
	Stream.WriteUInt32(DataCenter->Version);

	if (!DataCenter->Unk.Serialize(Stream)) {
		return false;
	}

	Stream.WriteU_int64(AttributeTypes.size());

	for (auto& Item : AttributeTypes) {
		Stream.WriteU_int64(Item.first.size() * 2);
		Stream.Write((uint8_t*)Item.first.data(), Item.first.size() * 2);
		Stream.WriteUInt16(Item.second);
	}

	std::wstring WRootDir = RootDir;
	std::string MetaFilePath = std::string(WRootDir.begin(), WRootDir.end());
	DataCenter->BuildDCExportName(MetaFilePath);

	MetaFilePath += "/DCMetadata.bin";
	if (!Stream.save_to_file(MetaFilePath.c_str())) {
		Message("Failed to Save %s file", MetaFilePath.c_str());
	}

	return true;
}

bool DCAdaptors::DCAdaptor::ReadMetadata(const wchar_t* File, INT& FilesCount) noexcept
{
	FIStream Stream;
	Stream._isLoading = true;

	if (!Stream.load_from_file(File)) {
		Message("DCAddaptor:: Could not find Metadata file");
		return false;
	}

	FilesCount = Stream.ReadInt32();

	DataCenter->FormatVersion = Stream.ReadInt32();
	DataCenter->Unk1_8 = Stream.ReadInt64();
	DataCenter->Version = Stream.ReadInt32();

	if (!DataCenter->Unk.Serialize(Stream)) {
		Message("DCAddaptor:: Failed to read Metadata file");
		return false;
	}

	wchar_t AttributeTypeNameBuffer[1024];

	size_t AttributeTypesSize = Stream.ReadUInt64();
	for (size_t i = 0; i < AttributeTypesSize; i++) {
		size_t StrLength = Stream.ReadUInt64();
		Stream.Read((uint8_t*)AttributeTypeNameBuffer, StrLength);
		AttributeTypeNameBuffer[StrLength] = L'\0';

		WORD AttrType = Stream.ReadUInt16();

		AttributeTypes.insert(std::pair<std::wstring, WORD>(AttributeTypeNameBuffer, AttrType));
	}

	Message("Read %lld Attribute types from metadata", AttributeTypes.size());

	return true;
}

bool DCAdaptors::DCAdaptor::FindAllFilesInDirectory(const char* DCDir, const char* extension, std::vector<std::string>& outFiles)const noexcept
{
	std::string ext(extension);
	for (auto& p : std::filesystem::recursive_directory_iterator(DCDir))
	{
		if (p.path().extension() == ext) {
			outFiles.push_back(p.path().string());
		}
	}

	return true;
}

wchar_t* FindChar(wchar_t* Buffer, wchar_t TargetChar) {
	while (*Buffer != TargetChar && *Buffer) {
		Buffer++;
	}

	if (*Buffer == (wchar_t)0) {
		return nullptr;
	}

	return Buffer;
}

wchar_t* ConsumeSpaces(wchar_t* Buffer) {
	while (*Buffer && (*Buffer == L' ' || *Buffer == L'\t'))
	{
		Buffer++;
	}

	if (*Buffer == (wchar_t)0) {
		return nullptr;
	}

	return Buffer;
}

wchar_t* FindElementNameEnding(wchar_t* Buffer) {
	static wchar_t Stoppers[] = { L'\"', L'>', L'/', L'=' };

	while (*Buffer) {
		for (auto& Stopper : Stoppers) {
			if (*Buffer == Stopper) {
				return nullptr;
			}
		}

		if (*Buffer == L' ')
		{
			return Buffer;
		}
	}

	return nullptr;
}

wchar_t* DCAdaptors::DCXMLAdaptor::ParseElement(wchar_t* Buffer) noexcept
{
	Buffer = FindChar(Buffer, L'<');
	if (!Buffer) {
		return nullptr;
	}

	Buffer = ConsumeSpaces(Buffer);
	if (!Buffer) {
		return nullptr;
	}

	wchar_t* NameEnding = FindElementNameEnding(Buffer);
	if (!NameEnding) {
		return nullptr;
	}

	auto Element = DataCenter->CreateNewElement(Buffer, (size_t)(NameEnding - Buffer));
	if (!Element) {
		return nullptr;
	}

	Element.NameCache = std::wstring(Buffer, (size_t)(NameEnding - Buffer));

	Buffer = ConsumeSpaces(Buffer);
	if (!Buffer) {
		return nullptr;
	}



	return nullptr;
}
