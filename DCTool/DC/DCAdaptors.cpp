#include "../App.h"

#include <filesystem>

#define _CRT_NON_CONFORMING_SWPRINTFS 1

#include <direct.h>
#include <stdio.h>
#include <unordered_map>
#include <string_view>

#include "../RapidXML/rapidxml.hpp"

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

void EscapeValue(std::wstring& String, bool reverse = false)noexcept {
	std::wstring rightSubstr;

	if (reverse) {
		/*start_reverse:
			size_t Size = String.size();

			while (Size--) {
				if (String[Size] == L'&') {
					rightSubstr = std::move(String.substr(Size + 1));

					if (rightSubstr.starts_with(L"&quot;")) {
						std::wstring temp;

						temp += String.substr(0, Size);
						temp += L"\"";
						temp += rightSubstr.substr(6);

						String = std::move(temp);

						goto start;
					}
					else if (rightSubstr.starts_with(L"&apos;")) {
						std::wstring temp;

						temp += String.substr(0, Size);
						temp += L"'";
						temp += rightSubstr.substr(6);

						String = std::move(temp);

						goto start;
					}
					else if (rightSubstr.starts_with(L"&amp;")) {
						std::wstring temp;

						temp += String.substr(0, Size);
						temp += L"&";
						temp += rightSubstr.substr(5);

						String = std::move(temp);

						goto start;
					}
					else if (rightSubstr.starts_with(L"&lt;")) {
						std::wstring temp;

						temp += String.substr(0, Size);
						temp += L"<";
						temp += rightSubstr.substr(4);

						String = std::move(temp);

						goto start;
					}
					else if (rightSubstr.starts_with(L"&gt;")) {
						std::wstring temp;

						temp += String.substr(0, Size);
						temp += L">";
						temp += rightSubstr.substr(4);

						String = std::move(temp);

						goto start;
					}
				}
			}*/
	}
	else {
	start:
		size_t Size = String.size();

		while (Size--) {
			if (String[Size] == L'"') {
				std::wstring temp;

				temp += String.substr(0, Size);
				temp += L"&quot;";
				temp += String.substr(Size + 1);

				String = std::move(temp);

				goto start;
			}
			else if (String[Size] == L'\'') {
				std::wstring temp;

				temp += String.substr(0, Size);
				temp += L"&apos;";
				temp += String.substr(Size + 1);

				String = std::move(temp);
				goto start;
			}
			else if (String[Size] == L'<') {
				std::wstring temp;

				temp += String.substr(0, Size);
				temp += L"&lt;";
				temp += String.substr(Size + 1);

				String = std::move(temp);
				goto start;
			}
			else if (String[Size] == L'>') {
				std::wstring temp;

				temp += String.substr(0, Size);
				temp += L"&gt;";
				temp += String.substr(Size + 1);

				String = std::move(temp);
				goto start;
			}
			else if (String[Size] == L'&') {
				rightSubstr = String.substr(Size + 1);
				if (rightSubstr.starts_with(L"lt;") ||
					rightSubstr.starts_with(L"gt;") ||
					rightSubstr.starts_with(L"quot;") ||
					rightSubstr.starts_with(L"apos;") ||
					rightSubstr.starts_with(L"amp;")) {
					continue;
				}

				std::wstring temp;

				temp += String.substr(0, Size);
				temp += L"&amp;";
				temp += String.substr(Size + 1);

				String = std::move(temp);

				goto start;
			}
		}
	}
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
			//Message("AttrFullName e[%ws] a[%ws] -> [%ws]", Element->NameRef.Get(), Attribute->NameRef.Get(), AttrFullName.c_str());
			AttributeTypes.insert(std::pair<std::wstring, WORD>(AttrFullName, (WORD)Attribute->TypeInfo));
		}

		*(Buffer++) = L'=';
		*(Buffer++) = L'\"';

		EscapeValue(Attribute->StringyfiedValue);

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

	constexpr size_t CBufferSize = (1024 * 1024) * 512;

	auto buffer = std::unique_ptr<wchar_t>(new wchar_t[CBufferSize]);
	auto utf8Buffer = std::unique_ptr<char>(new char[(CBufferSize * 2) + 1]);

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

		buffer.get()[0] = '\0';
		wchar_t* end = XmlFromDCElement(Element, buffer.get());
		if (end == buffer.get()) {
			continue;
		}

		*end = L'\0';

		INT EndIndex = WideCharToMultiByte(CP_UTF8, 0, buffer.get(), (end - buffer.get()), utf8Buffer.get(), (CBufferSize * 2) + 1, NULL, NULL);
		if (!EndIndex) {
			Message("Failed to convert to UTF8");
			return false;
		}

		utf8Buffer.get()[EndIndex] = '\0';

		std::ofstream outputFile = std::ofstream(FileSavePath.c_str(), std::ofstream::binary);

		outputFile.write(utf8Buffer.get(), EndIndex);

		outputFile.close();
	}

	if (!SerializeMetadata(RootDir, ValidCount)) {
		Message("Failed to save metadata");
		return false;
	}

	Message("DC exported to XML successfully");

	return true;
}

bool DCAdaptors::DCXMLAdaptor::BuildDC(const wchar_t* DirName, TRef<S1DataCenter::S1DataCenter> DataCenter)
{
	S1DataCenter::StateGuard<S1DataCenter::DCState::Building> StateGuard(DataCenter.Get());

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

	ElementItemRaw RootElement;
	RootElement.Name = L"__root__";

	constexpr size_t ReadBufferSize = 1024 * 1024 * 512;
	auto Utf8Buffer = std::unique_ptr<char>(new char[ReadBufferSize + 1]);

	for (size_t Index = 0; Index < FileNames.size(); Index++)
	{
		std::ifstream File = std::ifstream(FileNames[Index].c_str(), std::ifstream::binary | std::ifstream::ate);
		if (!File.is_open()) {
			Message("Failed to open xml file[%s]", FileNames[Index].c_str());
			return false;
		}

		size_t StreamSize = File.tellg();
		File.seekg(0, std::ifstream::beg);

		if (StreamSize == 0) {
			Message("File[%s] is empty", FileNames[Index].c_str());
			File.close();
			continue;
		}

		if (StreamSize >= ReadBufferSize) {
			Message("File[%s] exceeds read buffer size[%lld]", ReadBufferSize);
			File.close();
			return false;
		}

		File.read(Utf8Buffer.get(), StreamSize);
		File.close();

		Utf8Buffer.get()[StreamSize] = '\0';

		if (!ParseXmlFile(FileNames[Index].c_str(), Utf8Buffer.get(), StreamSize, &RootElement)) {
			Message("Failed to parse xml file[%s]", FileNames[Index].c_str());
			return false;
		}
	}

	if (!DataCenter->InsertElementTree(&RootElement)) {
		Message("Failed to Build DC from xml data");
		return false;
	}

	return true;
}

bool PrepareXMLTree(const char* FileName, rapidxml::xml_node<>* xmlNode, ElementItemRaw* RawElement)noexcept {

	//RawElement->Children.push_back(std::move(ElementItemRaw()));

	//ElementItemRaw* NewElement = &RawElement->Children.back();

	constexpr size_t CConvertStrSize = (1024 * 1024) * 64;

	std::string_view ConvertView = xmlNode->name();
	size_t ConvertSize = ConvertView.size();
	static wchar_t ConvertStr[CConvertStrSize];

	INT EndLen = MultiByteToWideChar(CP_UTF8, 0, ConvertView.data(), ConvertSize, ConvertStr, 4096);
	if (!EndLen) {
		Message("Failed to convert element name to UTF16 File:[%s] name[%s]", FileName, ConvertView.data());
		return false;
	}
	ConvertStr[EndLen] = L'\0';

	RawElement->Name = std::move(ConvertStr);

	//Prepare attributes
	for (rapidxml::xml_attribute<>* attr = xmlNode->first_attribute(); attr != NULL; attr = attr->next_attribute())
	{
		ConvertView = attr->name();
		ConvertSize = ConvertView.size();

		EndLen = EndLen = MultiByteToWideChar(CP_UTF8, 0, ConvertView.data(), ConvertSize, ConvertStr, 4096);
		if (!EndLen) {
			Message("Failed to convert element name to UTF16 File:[%s] name[%s]", FileName, ConvertView.data());
			return false;
		}
		ConvertStr[EndLen] = L'\0';

		std::wstring AttributeFullName = RawElement->Name;
		AttributeFullName += L'.';
		AttributeFullName += ConvertStr;

		auto Type = AttributeTypes.find(AttributeFullName);
		if (Type == AttributeTypes.end()) {
			Message("Could not find type for attribute [%ws] in File [%s]", AttributeFullName.c_str(), FileName);
			return false;
		}

		RawElement->Attributes.push_back(std::move(AttributeItemRaw()));
		RawElement->Attributes.back().Name = std::move(ConvertStr);

		ConvertView = attr->value();
		ConvertSize = ConvertView.size();

		if (ConvertSize) {
			EndLen = MultiByteToWideChar(CP_UTF8, 0, ConvertView.data(), ConvertSize, ConvertStr, CConvertStrSize);
			if (!EndLen) {
				Message("Failed to convert attr value to UTF16 File:[%s] value[%s]", FileName, ConvertView.data());
				return false;
			}
			ConvertStr[EndLen] = L'\0';
		}
		else {
			ConvertStr[0] = L'\0';
		}

		RawElement->Attributes.back().Value = std::move(ConvertStr);
		RawElement->Attributes.back().Type = Type->second;
	}

	//Prepare children
	for (rapidxml::xml_node<>* child = xmlNode->first_node(); child != NULL; child = child->next_sibling())
	{
		RawElement->Children.push_back(std::move(ElementItemRaw()));

		if (!PrepareXMLTree(FileName, child, &RawElement->Children.back())) {
			Message("Failed to prepare node [%s] in File [%s]", child->name(), FileName);
			return false;
		}
	}

	return true;
}

bool DCAdaptors::DCXMLAdaptor::ParseXmlFile(const char* FileName, char* Buffer, size_t BufferLength, ElementItemRaw* Parent) noexcept
{
	auto XmlDoc = std::make_unique<rapidxml::xml_document<>>();

	XmlDoc->parse<0>(Buffer);

	if (XmlDoc->first_node()->next_sibling()) {
		Message("File [%s] must contain only one root node", FileName);
		return false;
	}

	Parent->Children.push_back(std::move(ElementItemRaw()));

	return PrepareXMLTree(FileName, XmlDoc->first_node(), &Parent->Children.back());
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

	auto AttributeTypeNameBuffer = std::unique_ptr<wchar_t>(new wchar_t[1024]);

	size_t AttributeTypesSize = Stream.ReadUInt64();
	for (size_t i = 0; i < AttributeTypesSize; i++) {
		size_t StrLength = Stream.ReadUInt64();
		Stream.Read((uint8_t*)AttributeTypeNameBuffer.get(), StrLength);

		WORD AttrType = Stream.ReadUInt16();

		AttributeTypes.insert(std::pair<std::wstring, WORD>(std::wstring(AttributeTypeNameBuffer.get(), StrLength / 2), AttrType));
	}

	auto result = AttributeTypes.find(L"String.id");
	if (result == AttributeTypes.end()) {

		Message("Read %lld Attribute types from metadata", AttributeTypes.size());
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

