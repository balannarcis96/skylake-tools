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

	if (!PrepareAttributes()) {
		return false;
	}

	if (!PrepareElements()) {
		return false;
	}

	return true;
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
				Attribute.NameRef = NamesMap.AllStrings[Attribute.NameId - 1].CachedString;
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
	for (auto& ElementsBlock : Elements.Data) {

		for (auto& Element : ElementsBlock.Data) {

			auto [pageIndex, pageOffset] = NamesMap.AllStrings.Data[Element.Name - 1].Indices;
			Element.NameRef = NamesMap.GetString(pageIndex, pageOffset);

			if (Element.ArgsCount) {
				for (int Index = 0; Index < Element.ArgsCount; Index++)
				{
					AttributeItem* SubAttributes = &(Attributes.Data[Element.ArgsIndices.first].Data[Element.ArgsIndices.second]);
					Element.Arguments.push_back(SubAttributes + Index);
				}
			}

			if (Element.ChildCount) {
				for (int Index = 0; Index < Element.ChildCount; Index++)
				{
					ElementItem* SubElements = &(Elements.Data[Element.ChildrenIndices.first].Data[Element.ChildrenIndices.second]);
					Element.Children.push_back(SubElements + Index);
				}
			}

			//Message("Element [name=%ws childCount=%lld argsCount=%lld]", Element.NameRef.Get(), Element.Children.size(), Element.Arguments.size());
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
