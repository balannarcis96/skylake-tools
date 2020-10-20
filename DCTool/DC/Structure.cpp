#include "../App.h"

#include <iomanip>
#include <tuple>

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

	if (!Stream.IsLoading()) {
		//Values map is not used by client !
		for (auto& buckets : ValuesMap.Buckets.Buckets) {
			buckets.Clear();
		}
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

	//Cache indices names
	for (size_t i = 0; i < Indices.Data.size(); i++) {
		if (Indices.Data[i].Key1) {
			Indices.Data[i].Name1 = NamesMap.AllStrings.Data[Indices.Data[i].Key1 - 1].CachedString;
		}
		else {
			continue;
		}

		if (Indices.Data[i].Key2) {
			Indices.Data[i].Name2 = NamesMap.AllStrings.Data[Indices.Data[i].Key2 - 1].CachedString;
		}
		else {
			continue;
		}

		if (Indices.Data[i].Key3) {
			Indices.Data[i].Name3 = NamesMap.AllStrings.Data[Indices.Data[i].Key3 - 1].CachedString;
		}
		else {
			continue;
		}

		if (Indices.Data[i].Key4) {
			Indices.Data[i].Name4 = NamesMap.AllStrings.Data[Indices.Data[i].Key4 - 1].CachedString;
		}
	}

	//Set root parent
	SetParents(GetRootElement().Get(), nullptr);

	BuildIndicesCache();

	return true;
}

bool S1DataCenter::S1DataCenter::BuildIndices() noexcept
{
	//1. Search for name attribute indices and add references to string entry
	for (size_t i = 0; i < Indices.Data.size(); i++) {
		auto& item = Indices.Data[i];

		if (item.Name1Cached.size()) {
			if ((item.Key1 = NamesMap.FindString(item.Name1Cached.c_str()), !item.Key1)) {
				Message("Failed to find indexed attribute named[%ws].", item.Name1Cached.c_str());
				return false;
			}
		}

		if (item.Name2Cached.size()) {
			if ((item.Key2 = NamesMap.FindString(item.Name2Cached.c_str()), !item.Key2)) {
				Message("Failed to find indexed attribute named[%ws].", item.Name2Cached.c_str());
				return false;
			}
		}

		if (item.Name3Cached.size()) {
			if ((item.Key3 = NamesMap.FindString(item.Name3Cached.c_str()), !item.Key3)) {
				Message("Failed to find indexed attribute named[%ws].", item.Name3Cached.c_str());
				return false;
			}
		}

		if (item.Name4Cached.size()) {
			if ((item.Key4 = NamesMap.FindString(item.Name4Cached.c_str()), !item.Key4)) {
				Message("Failed to find indexed attribute named[%ws].", item.Name4Cached.c_str());
				return false;
			}
		}

		if (item.Key1) {
			NamesMap.AllStrings.Data[item.Key1 - 1].RefIndices.push_back({ (WORD)i , 1 });

			for (auto& Element : NamesMap.AllStrings.Data[item.Key1 - 1].RefElements) {
				if (GetElement(Element)->Index) {
					Message("Found element index collision!");
					return false;
				}
				GetElement(Element)->SetIndex(i, 1);
			}
		}
		if (item.Key2) {
			NamesMap.AllStrings.Data[item.Key2 - 1].RefIndices.push_back({ (WORD)i , 2 });

			for (auto& Element : NamesMap.AllStrings.Data[item.Key1 - 1].RefElements) {
				if (GetElement(Element)->Index) {
					Message("Found element index collision!");
					return false;
				}
				GetElement(Element)->SetIndex(i, 1);
			}
		}
		if (item.Key3) {
			NamesMap.AllStrings.Data[item.Key3 - 1].RefIndices.push_back({ (WORD)i , 3 });

			for (auto& Element : NamesMap.AllStrings.Data[item.Key1 - 1].RefElements) {
				if (GetElement(Element)->Index) {
					Message("Found element index collision!");
					return false;
				}
				GetElement(Element)->SetIndex(i, 1);
			}
		}
		if (item.Key4) {
			NamesMap.AllStrings.Data[item.Key4 - 1].RefIndices.push_back({ (WORD)i , 4 });

			for (auto& Element : NamesMap.AllStrings.Data[item.Key1 - 1].RefElements) {
				if (GetElement(Element)->Index) {
					Message("Found element index collision!");
					return false;
				}
				GetElement(Element)->SetIndex(i, 1);
			}
		}
	}

	//2. Sort strings by (have indices ref) and then by name
	std::sort(NamesMap.AllStrings.Data.begin(), NamesMap.AllStrings.Data.end(),
		[](const DCMap::StringEntry& left, const DCMap::StringEntry& right)
		{
			const auto lhs_root = wcscmp(left.CachedString.Get(), L"__root__") != 0;
			const auto rhs_root = wcscmp(right.CachedString.Get(), L"__root__") != 0;

			const auto lhs_indices = left.RefIndices.size() == 0;
			const auto rhs_indices = right.RefIndices.size() == 0;

			return std::tie(
				lhs_root,
				lhs_indices)
				<
				std::tie(
					rhs_root,
					rhs_indices
				) /*? (std::wstring_view(left.CachedString.Get()) < std::wstring_view(right.CachedString.Get())) : false*/;
		});

	//3. Restore correct name indices
	for (size_t i = 0; i < NamesMap.AllStrings.Data.size(); i++) {

		for (auto& item : NamesMap.AllStrings.Data[i].RefAttributes) {
			GetAttribute(item)->NameId = (WORD)(i + 1);
		}

		for (auto& item : NamesMap.AllStrings.Data[i].RefElements) {
			GetElement(item)->Name = (WORD)(i + 1);
		}

		for (auto& item : NamesMap.AllStrings.Data[i].RefIndices) {
			if (item.second == 1) {
				Indices.Data[item.first].Key1 = (WORD)(i + 1);
			}
			else if (item.second == 2) {
				Indices.Data[item.first].Key2 = (WORD)(i + 1);
			}
			else if (item.second == 3) {
				Indices.Data[item.first].Key3 = (WORD)(i + 1);
			}
			else if (item.second == 4) {
				Indices.Data[item.first].Key4 = (WORD)(i + 1);
			}
		}
	}

	DumpIndices();

	return true;
}

void S1DataCenter::S1DataCenter::DumpIndices() const noexcept
{
	std::unordered_map<std::wstring, INT> excluded;

	std::wofstream file = std::wofstream("c:/indices.txt");

	for (size_t i = 0; i < Elements.Data.size(); i++) {
		for (size_t j = 0; j < Elements.Data[i].Data.size(); j++) {
			if (!Elements.Data[i].Data[j].IsValid()) {
				continue;
			}

			if (Elements.Data[i].Data[j].GetIndexValue() == 0) {
				continue;
			}

			auto Path = Elements.Data[i].Data[j].BuildPath();

			auto exists = excluded.find(Path);
			if (exists != excluded.end()) {
				continue;
			}

			excluded.insert({ Path, 1 });

			if (Path[0] != '_') {
				DebugBreak();
			}

			file << Path << std::setw(128) << std::setfill(L' ');
			file << Elements.Data[i].Data[j].GetIndexValue() << std::setw(10) << std::setfill(L' ');

			const auto& Index = Indices.Data[Elements.Data[i].Data[j].GetIndexValue()];

			file << Index.Key1 << std::setw(4) << std::setfill(L' ')
				<< Index.Key2 << std::setw(4) << std::setfill(L' ')
				<< Index.Key3 << std::setw(4) << std::setfill(L' ')
				<< Index.Key4 << std::setw(4) << std::setfill(L' ') << std::setw(10) << std::setfill(L' ');

			file << (Index.Key1 ? NamesMap.AllStrings.Data[Index.Key1 - 1].CachedString.Get() : L"") << std::setw(55) << std::setfill(L' ')
				<< (Index.Key2 ? NamesMap.AllStrings.Data[Index.Key2 - 1].CachedString.Get() : L"") << std::setw(55) << std::setfill(L' ')
				<< (Index.Key3 ? NamesMap.AllStrings.Data[Index.Key3 - 1].CachedString.Get() : L"") << std::setw(55) << std::setfill(L' ')
				<< (Index.Key4 ? NamesMap.AllStrings.Data[Index.Key4 - 1].CachedString.Get() : L"") << std::setw(55) << std::setfill(L' ');

			file << std::endl;
		}
	}
}

void S1DataCenter::S1DataCenter::DumpRootChildren()  noexcept
{
	std::wofstream file = std::wofstream("c:/root_children.txt");

	const ElementItem* Root = GetRootElement().Get();

	for (auto& item : Root->Children) {
		if (!item->IsValid()) {
			file << L"Invalid\n";
		}
		else {
			file << item->BuildPath() << '\n';
		}
	}

	file.close();
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
	//std::wofstream dupFile = std::wofstream("C:/duplicates.txt");
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
					auto ChildElement = GetElement({ Element.ChildrenIndices.first, Element.ChildrenIndices.second + Index });

					if (!ChildElement->Parent.Get()) {
						ChildElement->Parent = &Element;
						Element.Children.push_back(ChildElement);
					}
					else {
						ElementItem* NewItem = new ElementItem(*ChildElement);
						NewItem->IsDuplicated = true;

						NewItem->Parent = &Element;

						Element.Children.push_back(NewItem);
					}
				}
			}

			if (Element.IsIndexEnabled()) {
				Indices.Data[Element.GetIndexValue()].ElementsCount++;
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

std::wstring S1DataCenter::AttributeItem::BuildPath(const ElementItem* parent) const noexcept
{
	std::wstring outPath = parent->BuildPath();
	outPath += L'.';
	outPath += NameRef.Get();
	return std::move(outPath);
}