#include "../App.h"

DWORD appStrihash(const TCHAR* Data);

bool S1DataCenter::S1DataCenter::Serialize(FIStream& Stream)
{
	if (Stream.IsLoading()) {
		FormatVersion = Stream.ReadInt32();
		Unk1_8 = Stream.ReadInt64();
		Version = Stream.ReadInt32();

	}
	else {
		Stream.WriteInt32(FormatVersion);
		Stream.Write_int64(Unk1_8);
		Stream.WriteInt32(Version);
	}

	if (!Unk.Serialize(Stream)) {
		return false;
	}

	if (!Indices.Serialize(Stream)) {
		return false;
	}

	if (!Attributes.Serialize(Stream)) {
		return false;
	}

	if (!Elements.Serialize(Stream)) {
		return false;
	}

	if (!ValuesMap.Serialize(Stream)) {
		return false;
	}

	if (!NamesMap.Serialize(Stream)) {
		return false;
	}

	if (Stream.IsLoading()) {
		EndCount = Stream.ReadUInt32();
	}
	else {
		Stream.WriteUInt32(EndCount);
	}

	return true;
}

bool S1DataCenter::S1DataCenter::Prepare()
{
	if (Attributes.Data.size()) {
		for (size_t i = 0; i < Attributes.Data.size(); i++)
		{
			if (Attributes[i].Data.size()) {
				for (size_t j = 0; j < Attributes[i].Data.size(); j++)
				{
					if (Attributes[i].Data[j].IsString()) {

						const auto* String = ValuesMap.GetString(Attributes[i].Data[j].Indices.first, Attributes[i].Data[j].Indices.second);

						Attributes[i].Data[j].StringRef = String;
					}
					else {
						Attributes[i].Data[j].StringRef = nullptr;
					}

					Attributes[i].Data[j].NameRef = NamesMap.AllStrings[Attributes[i].Data[j].NameId].CachedString;

					Attributes[i].Data[j].MessageThis();
				}
			}
		}
	}

	if (Elements.Data.size()) {
		for (size_t i = 0; i < Elements.Data.size(); i++)
		{
			if (Elements.Data[i].Data.size()) {
				for (size_t j = 0; j < Elements.Data[i].Data.size(); j++)
				{
					auto& Data = Elements.Data[i].Data[j];

					Data.NameRef = NamesMap.AllStrings[Data.Name].CachedString;

					if (Data.ArgsIndices.first != UINT16_MAX || Data.ArgsIndices.second != UINT16_MAX) {
						Data.Arguments = &Attributes[Data.ArgsIndices.first].Data[Data.ArgsIndices.second];
					}
					else {
						Data.Arguments = nullptr;
					}

					if (Data.ChildrenIndices.first != UINT16_MAX && Data.ChildrenIndices.second != UINT16_MAX) {
						Data.Children = &Elements.Data[Data.ChildrenIndices.first].Data[Data.ChildrenIndices.second];
					}
					else {
						Data.Children = nullptr;
					}

					Data.MessageThis();
				}
			}
		}
	}

	DWORD Hash = 0;
	const auto& Item2 = ValuesMap.Buckets.GetElementEx(L"StrSheet_SystemMessage", Hash);

	return true;
}

bool S1DataCenter::S1DataCenter::PrepareAttributes()
{



	return true;
}

bool S1DataCenter::S1DataCenter::PrepareElements()
{
	return false;
}
