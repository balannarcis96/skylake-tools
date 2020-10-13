#pragma once

#include <unordered_map>
#include <array>

struct AttributeItemRaw {
	std::wstring		Name;
	std::wstring		Value;
	WORD				Type;

	AttributeItemRaw() {}
	AttributeItemRaw(AttributeItemRaw&& other) noexcept {
		Name = std::move(other.Name);
		Value = std::move(other.Value);
		Type = other.Type;
	}
	AttributeItemRaw(const AttributeItemRaw&) = delete;
	AttributeItemRaw& operator=(const AttributeItemRaw&) = delete;
	AttributeItemRaw& operator=(AttributeItemRaw&& other) noexcept {
		if (&other == this) {
			return *this;
		}

		Name = std::move(other.Name);
		Value = std::move(other.Value);
		Type = other.Type;

		return *this;
	}
};
struct ElementItemRaw {
	std::wstring Name;

	std::vector<AttributeItemRaw> Attributes;
	std::vector<ElementItemRaw> Children;

	ElementItemRaw() {}
	ElementItemRaw(ElementItemRaw&& other) noexcept {
		Name = std::move(other.Name);

		Attributes = std::move(other.Attributes);
		Children = std::move(other.Children);
	}
	ElementItemRaw(const ElementItemRaw&) = delete;
	ElementItemRaw& operator=(const ElementItemRaw&) = delete;
	ElementItemRaw& operator=(ElementItemRaw&& other) noexcept {
		if (&other == this) {
			return *this;
		}

		Name = std::move(other.Name);

		Attributes = std::move(other.Attributes);
		Children = std::move(other.Children);

		return *this;
	}
};

namespace S1DataCenter {
	constexpr size_t			CElementsBlockSize = 65536;
	constexpr size_t			CAttributesBlockSize = 65536;
	constexpr size_t			CStringsBlockSize = 65536;

#if DC_64
	constexpr size_t			CAttributeSize = 12;
	constexpr size_t			CElementSize = 24;
#else
	constexpr size_t			CAttributeSize = 8;
	constexpr size_t			CElementSize = 16;
#endif

	enum class DCState {
		Idle,
		Serializing,
		Preparing,
		Dumping,
		Building
	};

	template<DCState StartState, DCState ExitState = DCState::Idle>
	struct StateGuard {
		StateGuard(struct S1DataCenter* dc);
		~StateGuard();
	private:
		TRef<S1DataCenter> dc;
	};

	enum AttributeType : WORD {
		AttributeType_Invalid = 0,
		AttributeType_Int = 1,
		AttributeType_Float = 2,
		AttributeType_String = 3,
	};

	//
	// String CRC, case sensitive.
	//
	inline DWORD appStrCrc(const TCHAR* Data)
	{
		DWORD Hash = 0;
		while (*Data)
		{
			WORD Ch = *Data++;

			Hash = (((Hash >> 8)) ^ GCRCTable[(Hash ^ Ch) & 0xFF]);
			Hash = (((Hash >> 8)) ^ GCRCTable[(Hash ^ (Ch >> 8)) & 0xFF]);
		}

		return Hash >> 0;
	}

	//
	// String CRC, case insensitive.
	//
	inline DWORD appStrCrcCaps(const TCHAR* Data)
	{
		return appStrihash(Data);
	}

	struct NOVTABLE DCSerializable {
		virtual bool Serialize(FIStream& Stream) = 0;
	};

	//DCArray
	template<typename T, size_t BasePadd = 0>
	struct DCArray : DCSerializable {
		std::vector<T>		Data;
		UINT32				Count = 0;

		DCArray() {}
		DCArray(DCArray&& other)noexcept {
			Data = std::move(other.Data);
			Count = other.Count;

			other.Count = 0;
		}

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				Count = Stream.ReadUInt32() - BasePadd;

				Data.reserve(Count);

				for (size_t i = 0; i < Count; i++)
				{
					T Object;

					if (!Object.Serialize(Stream)) {
						continue;
					}

					Data.push_back(std::move(Object));
				}
			}
			else {
				Stream.WriteUInt32(Count + BasePadd);

				for (size_t i = 0; i < Data.size(); i++)
				{
					if (!Data[i].Serialize(Stream)) {
						return false;
					}
				}
			}

			return true;
		}
		void operator =(DCArray& other) {
			Data = std::move(other.Data);
			Count = other.Count;

			other.UsedCount = 0;
		}

		void Clear()noexcept {
			Data.clear();
			Count = 0;
		}
	};

	template<typename T, size_t ElementSize>
	struct DCBlockArray : DCSerializable {
		std::vector<T>		Data;
		UINT				UsedCount = 0;
		UINT				MaxCount = 0;

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {

				MaxCount = Stream.ReadUInt32();
				UsedCount = Stream.ReadUInt32();

				Data.reserve(UsedCount);

				for (size_t i = 0; i < UsedCount; i++)
				{
					T Object;

					if (!Object.Serialize(Stream)) {
						continue;
					}

					Data.push_back(std::move(Object));
				}
			}
			else {
				Stream.WriteUInt32(MaxCount);
				Stream.WriteUInt32(UsedCount);

				for (size_t i = 0; i < UsedCount; i++)
				{
					if (!Data[i].Serialize(Stream)) {
						return false;
					}
				}
			}

			const size_t Remaining = MaxCount - UsedCount;
			if (Stream.IsLoading())
			{
				if (Remaining) {
					Stream._pos += Remaining * ElementSize;
				}
			}
			else {
				T DefaultObject;

				for (size_t i = 0; i < Remaining; i++) {
					if (!DefaultObject.Serialize(Stream)) {
						return false;
					}
				}
			}

			return true;
		}

		void operator =(DCBlockArray& other) {
			Data = std::move(other.Data);
			MaxCount = other.MaxCount;
			UsedCount = other.UsedCount;

			other.UsedCount = 0;
			other.MaxCount = 0;
		}

		void Clear()noexcept {
			Data.clear();
			UsedCount = 0;
			MaxCount = 0;
		}

		bool CanWrite(UINT Count = 1)const noexcept {
			return MaxCount - UsedCount >= Count;
		}
	};

	//String Blocks
	struct StringBlock : DCSerializable {
		TRef<wchar_t>	Block = nullptr;
		INT				BlockSize = 0;
		INT				UsedSize = 0;

		StringBlock() {}
		StringBlock(StringBlock&& other)noexcept {
			Block = other.Block.Release();
			BlockSize = other.BlockSize;
			UsedSize = other.UsedSize;
		}

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				BlockSize = Stream.ReadInt32();
				UsedSize = Stream.ReadInt32();

				Block = new wchar_t[BlockSize];

				Stream.Read((UINT8*)Block.Get(), (size_t)BlockSize * 2);
			}
			else {
				Stream.WriteInt32(BlockSize);
				Stream.WriteInt32(UsedSize);

				Stream.Write((UINT8*)Block.Get(), (size_t)BlockSize * 2);
			}

			return true;
		};

		//Returns true and index into block where the inserted string starts
		//Returns false if is full
		bool InsertString(const wchar_t* String, size_t StringSize, WORD& OutIndex) noexcept {
			const auto Remaining = BlockSize - UsedSize;

			//Check if it can fit string
			if (Remaining < StringSize + 1) {
				return false;
			}

			wchar_t* ToReturn = Block.Get() + UsedSize;

			OutIndex = (WORD)UsedSize;

			if (StringSize) {
				memcpy_s(ToReturn, sizeof(wchar_t) * StringSize, String, sizeof(wchar_t) * StringSize);

				UsedSize += StringSize;
			}

			Block.Get()[UsedSize] = L'\0';

			UsedSize++;

			return ToReturn;
		}

		bool AllocateBlock(size_t BlockSize = CStringsBlockSize)noexcept {
			Block = new wchar_t[BlockSize];

			if (Block.Get()) {
				this->BlockSize = BlockSize;
				ZeroMemory(Block.Get(), sizeof(wchar_t) * BlockSize);
			}

			return Block.Get() != nullptr;
		}

		~StringBlock() {
			if (Block.Get()) {
				delete[] Block.Get();
				Block = nullptr;
			}
		}
	};

	using DCStringBlocks = DCArray<StringBlock>;

	template<typename T>
	using DCPaddedArray = DCArray<T, 1>;

	//Buckets
	template<typename T>
	struct DCBuckets : DCSerializable {
		DWORD										Count = 0;
		std::vector<T>								Buckets;

		TRef<DCStringBlocks>						Blocks = nullptr;

		virtual bool Serialize(FIStream& Stream) override {
			if (!Count) {
				return true;
			}

			if (Stream.IsLoading()) {
				Buckets.reserve(Count);

				for (size_t i = 0; i < Count; i++)
				{
					T Bucket;

					Bucket.Blocks = Blocks;

					if (!Bucket.Serialize(Stream)) {
						return false;
					}

					Buckets.push_back(std::move(Bucket));
				}
			}
			else {
				for (size_t i = 0; i < Count; i++)
				{
					if (!Buckets[i].Serialize(Stream)) {
						return false;
					}
				}
			}

			return true;
		}

		inline const T& GetBucket(const wchar_t* string) const noexcept {
			const auto Hash = MapString(string);
			const auto Index = GetMappedIndex(Hash, Count);

			return Buckets[Index];
		}
		inline const wchar_t* GetElement(const wchar_t* string) const noexcept {
			const auto Hash = Crc32(string);
			const auto Index = GetMappedIndex(Hash, Count);

			return Buckets[Index].GetElement(Hash);
		}
		inline const T& GetElementEx(const wchar_t* Data, DWORD& OutHash) const noexcept {
			OutHash = 0;

			while (*Data)
			{
				TCHAR Ch = *Data++;
				switch (Ch)
				{
				case 0x9c:
					Ch = 0x8c;
					break;
				case 0xd0:
				case 0xdf:
				case 0xf0:
				case 0xf7:
					break;
				case 0xff:
					Ch = 0x9f;
					break;
				default:
					if ((Ch >= 0x61 && Ch <= 0x7a) || Ch - 0xe0 <= 0x1e) Ch -= 32;
					break;
				}

				DWORD Temp = (((OutHash >> 8)) ^ GCRCTable[(OutHash ^ Ch) & 0xFF]);
				OutHash = (((Temp >> 8)) ^ GCRCTable[(Temp ^ (Ch >> 8)) & 0xFF]);
			}

			const DWORD Index = (((OutHash >> 16) ^ OutHash) & (Count - 1));

			return Buckets[Index];
		}

		inline const TRef<T> GetBucket(INT Index)const noexcept {
			return Buckets.size() ? &Buckets[Index] : nullptr;
		}
		inline const TRef<StringBlock> GetStringBlock(INT Index)const noexcept {
			return Blocks.Get() ? &Blocks->Data[Index] : nullptr;
		}

		void Clear() noexcept {
			Buckets.clear();
		}
	};

	struct StringsBucket : DCSerializable {
		struct BucketElement : DCSerializable {
			DWORD							Hash;
			DWORD							Length;
			DWORD							Id;
			std::pair<WORD, WORD>			Indices;

			//Ref
			TRef<const wchar_t>				CachedString = nullptr;

			BucketElement() {
				Hash = 0;
				Length = 0;
				Id = 0;
				Indices = { 0,0 };
			}
			BucketElement(const BucketElement& Other) noexcept {
				Hash = Other.Hash;
				Length = Other.Length;
				Id = Other.Id;
				Indices = Other.Indices;

				CachedString = Other.CachedString;
			}
			BucketElement& operator=(const BucketElement& Other) noexcept {
				if (&Other == this) {
					return *this;
				}

				Hash = Other.Hash;
				Length = Other.Length;
				Id = Other.Id;
				Indices = Other.Indices;

				CachedString = Other.CachedString;

				return *this;
			}

			virtual bool Serialize(FIStream& Stream) override {
				if (Stream.IsLoading()) {
					Hash = Stream.ReadUInt32();
					Length = Stream.ReadUInt32();
					Id = Stream.ReadUInt32();
					Indices.first = Stream.ReadUInt16();
					Indices.second = Stream.ReadUInt16();
				}
				else {
					Stream.WriteUInt32(Hash);
					Stream.WriteUInt32(Length);
					Stream.WriteUInt32(Id);

					Stream.WriteUInt16(Indices.first);
					Stream.WriteUInt16(Indices.second);
				}

				return true;
			}
		};

		DWORD										Count = 0;
		std::vector<BucketElement>					Elements;

		//Ref
		TRef<DCStringBlocks>						Blocks = nullptr;

		const wchar_t* GetElement(const DWORD Hash) const noexcept {
			if (!Count) {
				return nullptr;
			}

			for (size_t i = 0; i < Count; i++)
			{
				if (Elements[i].Hash == Hash) {
					return Elements[i].CachedString.Get();
				}
			}

			return nullptr;
		}

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				Count = Stream.ReadUInt32();

				Elements.reserve(Count);

				for (size_t i = 0; i < Count; i++)
				{
					BucketElement Element;

					if (!Element.Serialize(Stream)) {
						return false;
					}

					Elements.push_back(std::move(Element));
				}
			}
			else {
				Stream.WriteUInt32(Count);

				for (size_t i = 0; i < Count; i++)
				{
					if (!Elements[i].Serialize(Stream)) {
						return false;
					}
				}
			}

			return true;
		}

		StringsBucket() {}
		StringsBucket(StringsBucket&& other) noexcept {
			Count = other.Count;
			Elements = std::move(other.Elements);
			Blocks = other.Blocks;

			other.Count = 0;
		}
		StringsBucket& operator=(StringsBucket&& other) noexcept {
			if (&other == this) {
				return *this;
			}

			Count = other.Count;
			Elements = std::move(other.Elements);
			Blocks = other.Blocks;

			other.Count = 0;

			return *this;
		}

		StringsBucket(const StringsBucket& other) = delete;
		StringsBucket& operator=(const StringsBucket& other) = delete;


	};

	using DCStringsBuckets = DCBuckets<StringsBucket>;

	struct DCMap : DCSerializable {
		struct StringEntry : DCSerializable {
			std::pair<WORD, WORD>			Indices = { 0,0 };

			//Ref
			TRef<const wchar_t>				CachedString = nullptr;

			StringEntry() {}
			StringEntry(StringEntry&& other)noexcept {
				Indices = other.Indices;
				CachedString = other.CachedString;
			}
			virtual bool Serialize(FIStream& Stream) override {
				if (Stream.IsLoading()) {
					Indices.first = Stream.ReadUInt16();
					Indices.second = Stream.ReadUInt16();
				}
				else {
					Stream.WriteUInt16(Indices.first);
					Stream.WriteUInt16(Indices.second);
				}

				return true;
			}
		};

		DCStringBlocks				StringBlocks;
		DCStringsBuckets			Buckets;
		DCPaddedArray<StringEntry>	AllStrings;

		std::unordered_map<std::wstring, WORD> PresentStrings;
		std::unordered_map<std::wstring, std::pair<WORD, WORD>> PresentStringsBig;

		DCMap() {}

		virtual bool Serialize(FIStream& Stream) override {
			if (!StringBlocks.Serialize(Stream)) {
				return false;
			}

			if (!Buckets.Serialize(Stream)) {
				return false;
			}

			if (!AllStrings.Serialize(Stream)) {
				return false;
			}

			return true;
		}
		const wchar_t* GetString(WORD BlockIndex, WORD StringIndex) {
			const wchar_t* Block = StringBlocks.Data[BlockIndex].Block.Get();

			return &Block[StringIndex];
		}

		std::atomic<DWORD> asd;

		bool Prepare() noexcept {
			for (auto& Bucket : Buckets.Buckets) {
				for (auto& BucketElement : Bucket.Elements) {
					BucketElement.CachedString = &(StringBlocks.Data[BucketElement.Indices.first].Block.Get())[BucketElement.Indices.second];

					if (!_wcsicmp(BucketElement.CachedString.Get(), L"__root__")) {
						asd.store(BucketElement.Id);
					}
				}
			}

			for (auto& StringEntry : AllStrings.Data) {
				if (StringEntry.Indices.first == UINT16_MAX || StringEntry.Indices.second == UINT16_MAX) {
					StringEntry.CachedString = nullptr;
				}
				else {
					StringEntry.CachedString = &(StringBlocks.Data[StringEntry.Indices.first].Block.Get())[StringEntry.Indices.second];
				}
			}

			return true;
		}

		void Clear() noexcept {
			StringBlocks.Clear();
			Buckets.Clear();
			AllStrings.Clear();
		}

		bool InsertString(const wchar_t* String, size_t StringSize, WORD& StringId) noexcept {

			auto Item = PresentStrings.find(String);
			if (Item != PresentStrings.end()) {
				StringId = Item->second;
				return true;
			}

			std::pair<WORD, WORD> Indices;
			if (!InsertStringInBlock(String, StringSize, Indices)) {
				return false;
			}

			AllStrings.Data.push_back(StringEntry());

			StringId = (WORD)(AllStrings.Data.size() - 1);

			AllStrings.Count++;
			AllStrings.Data.back().Indices = Indices;
			AllStrings.Data.back().CachedString = GetString(Indices.first, Indices.second);

			PresentStrings.insert({ String, StringId });

			return true;
		}

		bool InsertString(const wchar_t* String, size_t StringSize, std::pair<WORD, WORD>& OutIndices) noexcept {
			auto Item = PresentStringsBig.find(String);
			if (Item != PresentStringsBig.end()) {
				OutIndices = Item->second;
				return true;
			}

			if (!InsertStringInBlock(String, StringSize, OutIndices)) {
				return false;
			}

			AllStrings.Data.push_back(StringEntry());

			//StringId = (WORD)(AllStrings.Data.size() - 1);

			AllStrings.Count++;
			AllStrings.Data.back().Indices = OutIndices;
			AllStrings.Data.back().CachedString = GetString(OutIndices.first, OutIndices.second);

			PresentStringsBig.insert({ String, OutIndices });

			return true;
		}

		bool BuildHashTables() noexcept {
			const DWORD Mask = Buckets.Buckets.size() - 1;
			DWORD Id = 1;

			for (auto& StringEntry : AllStrings.Data) {

				const DWORD Hash = appStrCrc(StringEntry.CachedString.Get());
				const DWORD Index = (Hash >> 16 ^ Hash) & Mask;

				Buckets.Buckets[Index].Elements.push_back(StringsBucket::BucketElement());

				auto& HashmapElement = Buckets.Buckets[Index].Elements.back();

				HashmapElement.CachedString = StringEntry.CachedString;
				HashmapElement.Indices = StringEntry.Indices;
				HashmapElement.Id = Id++;
				HashmapElement.Hash = Hash;
				HashmapElement.Length = wcslen(StringEntry.CachedString.Get()) + 1;
			}

			//sort all bucket elements by Hash
			for (size_t i = 0; i < Buckets.Buckets.size(); i++)
			{
				std::sort(Buckets.Buckets[i].Elements.begin(), Buckets.Buckets[i].Elements.end(),
					[](const StringsBucket::BucketElement& a, const StringsBucket::BucketElement& b) -> bool
					{
						return a.Hash > b.Hash;
					});
			}

			return true;
		}

	private:
		bool InsertStringInBlock(const wchar_t* String, size_t StringSize, std::pair<WORD, WORD>& OutIndices)noexcept {
			for (size_t i = 0; i < StringBlocks.Data.size(); i++) {
				if (StringBlocks.Data[i].InsertString(String, StringSize, OutIndices.second)) {

					OutIndices.first = (WORD)(i);

					return true;
				}
			}

			StringBlocks.Data.push_back(std::move(StringBlock()));
			StringBlocks.Count++;

			if (!StringBlocks.Data.back().AllocateBlock()) {
				Message("StringBlock::Failed to allocate block!");
				return false;
			}

			OutIndices.first = (WORD)(StringBlocks.Data.size() - 1);

			return StringBlocks.Data.back().InsertString(String, StringSize, OutIndices.second);
		}
	};

	struct DCIndex : DCSerializable {
		union {
			struct {
				UINT16 Key1;
				UINT16 Key2;
				UINT16 Key3;
				UINT16 Key4;
			};

			ULONG	Key = 0;
		};

		DCIndex() {}
		DCIndex(const DCIndex& other) {
			Key1 = other.Key1;
			Key2 = other.Key2;
			Key3 = other.Key3;
			Key4 = other.Key4;
		}
		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				Key = Stream.ReadUInt32();
			}
			else {
				Stream.WriteUInt32(Key);
			}

			return true;
		}
		void operator = (const DCIndex& other) {
			Key1 = other.Key1;
			Key2 = other.Key2;
			Key3 = other.Key3;
			Key4 = other.Key4;
		}
	};

	struct UnkTree : DCSerializable {
		struct Unk3Unk1_Item : DCSerializable {
			DWORD	Unk1 = 0;
			DWORD	Unk2 = 0;
			DWORD	Unk3 = 0;

			virtual bool Serialize(FIStream& Stream) override {
				if (Stream.IsLoading()) {
					Unk1 = Stream.ReadUInt32();
					Unk2 = Stream.ReadUInt32();
					Unk3 = Stream.ReadUInt32();
				}
				else {
					Stream.WriteUInt32(Unk1);
					Stream.WriteUInt32(Unk2);
					Stream.WriteUInt32(Unk3);
				}

				return true;
			};
			void operator = (const Unk3Unk1_Item& other) {
				Unk1 = other.Unk1;
				Unk2 = other.Unk2;
				Unk3 = other.Unk3;
			}
		};

		struct Unk3Unk3Unk1_Item : DCSerializable {
			WORD	Unk1 = 0;
			WORD	Unk2 = 0;
			DWORD	Unk3 = 0;

			DCArray<Unk3Unk1_Item>	Unk4;
			DCArray<Unk3Unk3Unk1_Item>	Unk5;

			virtual bool Serialize(FIStream& Stream) override {
				if (Stream.IsLoading()) {
					Unk1 = Stream.ReadUInt16();
					Unk2 = Stream.ReadUInt16();
					Unk3 = Stream.ReadUInt32();
				}
				else {
					Stream.WriteUInt16(Unk1);
					Stream.WriteUInt16(Unk2);
					Stream.WriteUInt32(Unk3);
				}

				if (!Unk4.Serialize(Stream)) {
					return false;
				}

				if (!Unk5.Serialize(Stream)) {
					return false;
				}

				return true;
			};
			void operator = (const Unk3Unk1_Item& other) {
				Unk1 = other.Unk1;
				Unk2 = other.Unk2;
				Unk3 = other.Unk3;
			}
		};

		WORD								Unk1 = 0;
		WORD								Unk2 = 0;
		DWORD								Unk3 = 0;
		DCArray<Unk3Unk1_Item>				Unk3Unk1;
		DCArray<Unk3Unk3Unk1_Item>			Unk3Unk3;

		void Clear() noexcept {
			Unk1 = 0;
			Unk2 = 0;
			Unk3 = 0;
			Unk3Unk1.Clear();
			Unk3Unk3.Clear();
		}

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {

				Unk1 = Stream.ReadUInt16();
				Unk2 = Stream.ReadUInt16();
				Unk3 = Stream.ReadUInt32();

			}
			else {
				Stream.WriteUInt16(Unk1);
				Stream.WriteUInt16(Unk2);
				Stream.WriteUInt32(Unk3);
			}

			if (!Unk3Unk1.Serialize(Stream)) {
				return false;
			}

			if (!Unk3Unk3.Serialize(Stream)) {
				return false;
			}

			return true;
		}
	};

	struct AttributeItem : DCSerializable {
		WORD							NameId = 0;
		WORD							TypeInfo = 0;

		union {
			struct {
				std::pair<WORD, WORD>	Indices;
			};
			struct {
				INT						IntValue;
			};
			struct {
				float					FloatValue;
			};
		};

#if DC_64
		DWORD							Padding = 0;
#endif

		std::wstring					StringyfiedValue;

		//For Building
		size_t							Index = 0;
		std::wstring					NameCache;
		std::pair<WORD, WORD>			LocationCache;

		bool IsValid() const noexcept {
			return (TypeInfo & 3) != AttributeType_Invalid;
		}
		bool IsInt() const noexcept {
			return (TypeInfo & 3) == AttributeType_Int;
		}
		bool IsFloat() const noexcept {
			return (TypeInfo & 3) == AttributeType_Float;
		}
		bool IsString() const noexcept {
			return (TypeInfo & 3) == AttributeType_String;
		}

		TRef<StringBlock>		BlockRef = nullptr;
		TRef<const wchar_t>		StringRef = nullptr;
		TRef<const wchar_t>		NameRef = nullptr;

		AttributeItem() {
			IntValue = 0;
		}
		AttributeItem(const AttributeItem& other) {
			NameId = other.NameId;
			TypeInfo = other.TypeInfo;

			Indices = other.Indices;
#if DC_64
			Padding = other.Padding;
#endif
			BlockRef = other.BlockRef;
			StringRef = other.StringRef;

			NameCache = other.NameCache;
			LocationCache = other.LocationCache;
			Index = other.Index;
		}
		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				NameId = Stream.ReadUInt16();

				TypeInfo = (AttributeType)Stream.ReadUInt16();

				IntValue = Stream.ReadUInt32();

#if DC_64
				Padding = Stream.ReadUInt32(); //padding bytes
#endif
			}
			else {
				Stream.WriteUInt16(NameId);
				Stream.WriteUInt16((WORD)TypeInfo);

				Stream.WriteUInt16(Indices.first);
				Stream.WriteUInt16(Indices.second);

#if DC_64
				Stream.WriteUInt32(Padding);
#endif
			}

			return true;
		}
		virtual void operator=(AttributeItem& other) {
			NameId = other.NameId;
			TypeInfo = other.TypeInfo;

			Indices = other.Indices;
#if DC_64
			Padding = other.Padding;
#endif
			BlockRef = other.BlockRef;
			StringRef = other.StringRef;

			NameCache = other.NameCache;
			LocationCache = other.LocationCache;
			Index = other.Index;
		}

		bool CacheValue()noexcept {
			if (IsFloat()) {
				StringyfiedValue = std::move(std::to_wstring(FloatValue));
			}
			else if (IsInt()) {
				StringyfiedValue = std::move(std::to_wstring(IntValue));
			}
			else if (IsString()) {
				StringyfiedValue = std::move(std::wstring(StringRef.Get()));
			}
			else {
				Message("XmlNodeFromDCElement::Unknown attribute type %d", (INT)TypeInfo);
				return false;
			}

			return true;
		}
	};

	struct ElementItem : DCSerializable {
		WORD								Name = 0;
		WORD								Index = 0;
		WORD								ArgsCount = 0;
		WORD								ChildCount = 0;
		std::pair<WORD, WORD>				ArgsIndices = { UINT16_MAX,UINT16_MAX };
		std::pair<WORD, WORD>				ChildrenIndices = { UINT16_MAX,UINT16_MAX };

#if DC_64
		DWORD								Padd1 = 0;
		DWORD								Padd2 = 0;
#endif

		//Cached
		TRef<const wchar_t>					NameRef = nullptr;
		std::vector<AttributeItem*>			Attributes;
		std::vector<ElementItem*>			Children;

		std::wstring						NameCache;
		std::pair<WORD, WORD>				LocationCache;

		ElementItem() {}
		ElementItem(const ElementItem& other) {
			Name = other.Name;

			//Message("Element: nameId:%hd", Name);

			Index = other.Index;
			ChildCount = other.ChildCount;
			ArgsCount = other.ArgsCount;

			ArgsIndices = other.ArgsIndices;
			ChildrenIndices = other.ChildrenIndices;

#if DC_64
			Padd1 = other.Padd1;
			Padd2 = other.Padd2;
#endif

			Attributes = other.Attributes;
			Children = other.Children;

			NameCache = other.NameCache;
			LocationCache = other.LocationCache;
		}

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				Name = Stream.ReadUInt16();

				Index = Stream.ReadUInt16();
				ArgsCount = Stream.ReadUInt16();
				ChildCount = Stream.ReadUInt16();

				ArgsIndices.first = Stream.ReadUInt16();
				ArgsIndices.second = Stream.ReadUInt16();

#if DC_64
				Padd1 = Stream.ReadUInt32();
#endif

				ChildrenIndices.first = Stream.ReadUInt16();
				ChildrenIndices.second = Stream.ReadUInt16();

#if DC_64
				Padd2 = Stream.ReadUInt32();
#endif
			}
			else {
				Stream.WriteUInt16(Name);
				Stream.WriteUInt16(Index);
				Stream.WriteUInt16(ArgsCount);
				Stream.WriteUInt16(ChildCount);

				Stream.WriteUInt16(ArgsIndices.first);
				Stream.WriteUInt16(ArgsIndices.second);
#if DC_64
				Stream.WriteUInt32(Padd1);
#endif

				Stream.WriteUInt16(ChildrenIndices.first);
				Stream.WriteUInt16(ChildrenIndices.second);
#if DC_64
				Stream.WriteUInt32(Padd2);
#endif
			}

			return true;
		}

		virtual void operator=(ElementItem& other) {
			Name = other.Name;
			Index = other.Index;
			ChildCount = other.ChildCount;
			ArgsCount = other.ArgsCount;

			ArgsIndices = other.ArgsIndices;
			ChildrenIndices = other.ChildrenIndices;

#if DC_64
			Padd1 = other.Padd1;
			Padd2 = other.Padd2;
#endif

			Attributes = other.Attributes;
			Children = other.Children;

			NameCache = other.NameCache;
			LocationCache = other.LocationCache;
		}

		std::vector<const ElementItem*> GetChildren(const struct S1DataCenter* DC) const noexcept;
	};

	struct S1DataCenter : DCSerializable {
		INT														FormatVersion = 0;
		UINT64													Unk1_8 = 0;
		INT														Version = 0;

		UnkTree													Unk;
		DCArray<DCIndex>										Indices;
		DCArray<DCBlockArray<AttributeItem, CAttributeSize>>	Attributes;
		DCArray<DCBlockArray<ElementItem, CElementSize>>		Elements;
		DCMap													ValuesMap;
		DCMap													NamesMap;

		DWORD													EndCount = 0;

		S1DataCenter() {
			ValuesMap.Buckets.Count = 1024;
			NamesMap.Buckets.Count = 512;
		}

		bool PrepareForBuild()noexcept {
			for (size_t i = 0; i < ValuesMap.Buckets.Count; i++) {
				ValuesMap.Buckets.Buckets.push_back(StringsBucket());
			}

			for (size_t i = 0; i < NamesMap.Buckets.Count; i++) {
				NamesMap.Buckets.Buckets.push_back(StringsBucket());
			}

			return true;
		}

		virtual bool Serialize(FIStream& Stream) override;

		TRef<ElementItem> GetRootElement() {
			return &Elements.Data[0].Data[0];
		}

		bool Prepare();
		void Clear() {
			Unk.Clear();
			Indices.Clear();
			Attributes.Clear();
			Elements.Clear();
			ValuesMap.Clear();
			NamesMap.Clear();

			bIsLoaded = false;
			state = DCState::Idle;

			ElementsIndex = 0;
			AttributesIndex = 0;
		}

		inline const char* GetDCToolStateStr()noexcept {
			/*switch (state)
			{
			case DCState::Idle:
				return "Idle";
			case DCState::Serializing:
				return "Serializing";
			case DCState::Preparing:
				return "Preparing";
			case DCState::Dumping:
				return "Dumping";
			}*/

			return bIsLoaded ? "Loaded" : "Not Loaded";
		}

		bool IsLoaded() const noexcept {
			return bIsLoaded;
		}

		void BuildDCExportName(std::wstring& str) const noexcept {
			str += L"DC";
			str += L"_";
			str += std::to_wstring(Version);
		}
		void BuildDCExportName(std::string& str) const noexcept {
			str += "DC";
			str += "_";
			str += std::to_string(Version);
		}

		ElementItem* GetElement(std::pair<WORD, WORD> indices)noexcept {
			return &Elements.Data[indices.first].Data[indices.second];
		}
		ElementItem* GetElement(WORD blockIndex, WORD itemIndex)noexcept {
			return &Elements.Data[blockIndex].Data[itemIndex];
		}
		AttributeItem* GetAttribute(std::pair<WORD, WORD> indices)noexcept {
			return &Attributes.Data[indices.first].Data[indices.second];
		}
		AttributeItem* GetAttribute(WORD blockIndex, WORD itemIndex)noexcept {
			return &Attributes.Data[blockIndex].Data[itemIndex];
		}

		bool InsertElementTree(ElementItemRaw* Root)noexcept {
			std::pair<WORD, WORD> LocalRootIndices;
			if (!AllocateElement(*Root, LocalRootIndices)) {
				return false;
			}

			GetElement(LocalRootIndices)->ChildCount = (WORD)(Root->Children.size());

			if (Root->Children.size()) {
				std::pair<WORD, WORD> childrenIndices;
				if (!AllocateElements(Root->Children, childrenIndices)) {
					return false;
				}

				GetElement(LocalRootIndices)->ChildrenIndices = childrenIndices;

				//Parse all sub tree
				for (size_t i = 0; i < Root->Children.size(); i++) {
					if (!InsertElementTreeImpl(&Root->Children[i], std::pair<WORD, WORD>(childrenIndices.first, childrenIndices.second + i))) {
						return false;
					}
				}
			}

			if (!ValuesMap.BuildHashTables()) {
				return false;
			}

			if (!NamesMap.BuildHashTables()) {
				return false;
			}

			return true;
		}

	private:
		bool InsertElementTreeImpl(ElementItemRaw* RawElement, std::pair<WORD, WORD> ElementIndices)noexcept {
			if (RawElement->Attributes.size()) {
				if (!AllocateAttributes(RawElement->Attributes, GetElement(ElementIndices)->ArgsIndices)) {
					return false;
				}
			}

			GetElement(ElementIndices)->ArgsCount = (WORD)(RawElement->Attributes.size());
			GetElement(ElementIndices)->ChildCount = (WORD)(RawElement->Children.size());

			if (RawElement->Children.size()) {
				std::pair<WORD, WORD> childrenIndices;

				if (!AllocateElements(RawElement->Children, childrenIndices)) {
					return false;
				}

				GetElement(ElementIndices)->ChildrenIndices = childrenIndices;

				for (size_t i = 0; i < RawElement->Children.size(); i++) {
					if (!InsertElementTreeImpl(&RawElement->Children[i], std::pair<WORD, WORD>(childrenIndices.first, childrenIndices.second + i))) {
						return false;
					}
				}
			}

			return true;
		}

		bool AllocateElements(std::vector<ElementItemRaw>& RawElements, std::pair<WORD, WORD>& OutIndices)noexcept {
			auto& Block = GetElementsContainerForNewElements((INT)RawElements.size(), OutIndices.first);

			//start of this elements block
			OutIndices.second = (WORD)(Block.Data.size());

			for (auto& RawElement : RawElements) {

				Block.Data.push_back(ElementItem());
				Block.UsedCount++;

				//Prepare Element
				ElementItem* NewElement = &Block.Data.back();

				NewElement->Index = ++ElementsIndex;
				NewElement->LocationCache.first = OutIndices.first;
				NewElement->LocationCache.second = (WORD)(Block.Data.size() - 1);
				NewElement->NameCache = std::move(RawElement.Name);

				WORD NameId;
				if (!NamesMap.InsertString(NewElement->NameCache.data(), NewElement->NameCache.size(), NameId)) {
					Message("Failed to insert string [%ws]", NewElement->NameCache.c_str());
					return false;
				}

				NewElement->Name = NameId;
			}

			return true;
		}
		bool AllocateAttributes(std::vector<AttributeItemRaw>& RawAttributes, std::pair<WORD, WORD>& OutIndices)noexcept {
			auto& Block = GetAttributesContainerForNewAttributes((INT)RawAttributes.size(), OutIndices.first);

			//start of this attributes block
			OutIndices.second = (WORD)(Block.Data.size());

			for (auto& RawAttribute : RawAttributes) {
				Block.Data.push_back(AttributeItem());
				Block.UsedCount++;

				//Prepare Attribute
				AttributeItem* NewAttribute = &Block.Data.back();

				NewAttribute->Index = ++AttributesIndex;
				NewAttribute->LocationCache.first = OutIndices.first;
				NewAttribute->LocationCache.second = (WORD)(Block.Data.size() - 1);
				NewAttribute->NameCache = std::move(RawAttribute.Name);
				NewAttribute->StringyfiedValue = std::move(RawAttribute.Value);
				NewAttribute->TypeInfo = RawAttribute.Type;

				WORD StringId;

				if (RawAttribute.Type == AttributeType_String) {
					DWORD Crc32 = appStrCrcCaps(NewAttribute->NameCache.c_str());
					NewAttribute->TypeInfo = (WORD)(((Crc32 << 2) | AttributeType_String) & 0xffff);

					if (!ValuesMap.InsertString(NewAttribute->StringyfiedValue.data(), NewAttribute->StringyfiedValue.size(), NewAttribute->Indices)) {
						return false;
					}
				}
				else if (RawAttribute.Type == AttributeType_Int) {
					NewAttribute->IntValue = _wtol(NewAttribute->StringyfiedValue.c_str());
				}
				else if (RawAttribute.Type == AttributeType_Float) {
					NewAttribute->FloatValue = _wtof(NewAttribute->StringyfiedValue.c_str());
				}

				if (!NamesMap.InsertString(NewAttribute->NameCache.data(), NewAttribute->NameCache.size(), StringId)) {
					return false;
				}

				NewAttribute->NameId = StringId;
			}

			return true;
		}
		bool AllocateElement(ElementItemRaw& RawElement, std::pair<WORD, WORD>& OutIndices) {
			auto& Block = GetElementsContainerForNewElements(1, OutIndices.first);

			//start of this elements block
			OutIndices.second = (WORD)(Block.Data.size());

			Block.Data.push_back(ElementItem());
			Block.UsedCount++;

			//Prepare Element
			ElementItem* NewElement = &Block.Data.back();

			NewElement->Index = ++ElementsIndex;
			NewElement->LocationCache.first = OutIndices.first;
			NewElement->LocationCache.second = (WORD)(Block.Data.size() - 1);
			NewElement->NameCache = std::move(RawElement.Name);

			WORD NameId;
			if (!NamesMap.InsertString(NewElement->NameCache.data(), NewElement->NameCache.size(), NameId)) {
				Message("AllocateElement::Failed to insert string [%ws]", NewElement->NameCache.size());
				return false;
			}

			NewElement->Name = NameId;

			return true;
		}

		DCBlockArray<ElementItem, CElementSize>& GetElementsContainerForNewElements(INT Count, WORD& Out_Index)noexcept {
			//Try to find block that will contain all elements
			for (size_t i = 0; i < Elements.Data.size(); i++)
			{
				if (Elements.Data[i].CanWrite(Count)) {

					Out_Index = (WORD)(i);

					return Elements.Data[i];
				}
			}

			//Create new block array and reserve max
			Elements.Data.push_back(DCBlockArray<ElementItem, CElementSize>());
			Elements.Data.back().Data.reserve(CElementsBlockSize);
			Elements.Data.back().MaxCount = CElementsBlockSize;

			Elements.Count++;

			Out_Index = (WORD)(Elements.Data.size() - 1);

			return Elements.Data.back();
		}
		DCBlockArray<AttributeItem, CAttributeSize>& GetAttributesContainerForNewAttributes(INT Count, WORD& Out_Index)noexcept {
			//Try to find block that will contain all attributes
			for (size_t i = 0; i < Attributes.Data.size(); i++)
			{
				if (Attributes.Data[i].CanWrite(Count)) {

					Out_Index = (WORD)(i);

					return Attributes.Data[i];
				}
			}

			Out_Index = (WORD)(Attributes.Data.size());

			//Create new block array and reserve max
			Attributes.Data.push_back(DCBlockArray<AttributeItem, CAttributeSize>());

			Attributes.Data.back().Data.reserve(CAttributesBlockSize);
			Attributes.Data.back().MaxCount = CAttributesBlockSize;

			Attributes.Count++;

			return Attributes.Data.back();

		}

		bool PrepareAttributes();
		bool PrepareElements();

		bool				bIsLoaded = false;
		DCState				state = DCState::Idle;

		static inline INT	ElementsIndex = 0;
		static inline INT	AttributesIndex = 0;

		template<DCState StartState, DCState ExitState> friend struct StateGuard;
	};

	template<DCState StartState, DCState ExitState>
	inline StateGuard<StartState, ExitState>::StateGuard(S1DataCenter* dc)
	{
		this->dc = dc;
		dc->state = StartState;
	}

	template<DCState StartState, DCState ExitState>
	inline StateGuard<StartState, ExitState>::~StateGuard()
	{
		dc->state = ExitState;
		dc = nullptr;
	}
}