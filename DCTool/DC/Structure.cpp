#include "../App.h"

DWORD appStrihash(const TCHAR* Data);

bool S1DataCenter::S1DataCenter::Serialize(FIStream& Stream)
{
	StateGuard<DCState::Serializing> stateGuard(this);

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

	if (Stream.IsLoading()) {
		bIsLoaded = true;
	}

	return true;
}

bool S1DataCenter::S1DataCenter::Prepare()
{
	StateGuard<DCState::Preparing> stateGuard(this);

	if (!ValuesMap.Prepare()) {
		return false;
	}

	if (!NamesMap.Prepare()) {
		return false;
	}

	if (!PrepareAttributes()) {
		return false;
	}

	if (!PrepareElements()) {
		return false;
	}

	return true;
}

S1DataCenter::ElementItem* S1DataCenter::S1DataCenter::CreateNewElement(const wchar_t* Name, size_t NameSize) noexcept
{



	return nullptr;
}

S1DataCenter::AttributeItem* S1DataCenter::S1DataCenter::CreateNewAttribute(const wchar_t* Name, size_t NameSize, const wchar_t* Value, size_t ValueSize) noexcept
{
	return nullptr;
}

bool S1DataCenter::S1DataCenter::PrepareAttributes()
{
	for (auto& AttributesBlock : Attributes.Data) {

		for (auto& Attribute : AttributesBlock.Data) {
			if (Attribute.IsString()) {
				Attribute.StringRef = ValuesMap.GetString(Attribute.Indices.first, Attribute.Indices.second);
			}
			else {
				Attribute.StringRef = nullptr;
			}
			if (Attribute.NameId) {
				Attribute.NameRef = NamesMap.AllStrings.Data[Attribute.NameId - 1].CachedString;
			}
			else {
				Message("Attribute with name Id 0 ??");
			}
		}

	}

	return true;
}

bool S1DataCenter::S1DataCenter::PrepareElements()
{
	std::atomic<ElementItem*> LastItem = nullptr;
	for (auto& ElementsBlock : Elements.Data) {

		for (auto& Element : ElementsBlock.Data) {

			if (Element.Name == 0) {
				continue;
			}

			auto [pageIndex, pageOffset] = NamesMap.AllStrings.Data[Element.Name - 1].Indices;
			Element.NameRef = NamesMap.GetString(pageIndex, pageOffset);
			if (!Element.NameRef.Get()) {
				Message("Element name is null");
			}

			if (Element.ArgsCount) {
				for (size_t Index = 0; Index < Element.ArgsCount; Index++)
				{
					Element.Attributes.push_back(&(Attributes.Data[Element.ArgsIndices.first].Data[(size_t)Element.ArgsIndices.second + Index]));
				}
			}

			if (Element.ChildCount) {
				for (size_t Index = 0; Index < Element.ChildCount; Index++)
				{
					Element.Children.push_back(&(Elements.Data[Element.ChildrenIndices.first].Data[(size_t)Element.ChildrenIndices.second + Index]));
				}
			}
		}
	}

	return true;
}

std::vector<const S1DataCenter::ElementItem*> S1DataCenter::ElementItem::GetChildren(const S1DataCenter* DC) const noexcept
{
	std::vector<const ElementItem*> Out;

	if (!ChildCount) {
		return std::move(Out);
	}

	const auto* Start = &DC->Elements.Data[ChildrenIndices.first].Data[ChildrenIndices.second];

	for (size_t i = 0; i < ChildCount; i++)
	{
		Out.push_back(Start + i);
	}

	return std::move(Out);
}
