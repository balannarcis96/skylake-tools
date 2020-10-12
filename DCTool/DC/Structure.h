#pragma once

#include <unordered_map>
#include <array>
#include <stack>

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
	constexpr size_t			CStringBucketSize = 65536;

	constexpr size_t			CMaxStringBlocks = 512;
	constexpr size_t			CMaxStringBuckets = 512;

	constexpr size_t			CMaxIndices = 1024;
	constexpr size_t			CMaxAttributes = 65536 * 2;
	constexpr size_t			CMaxElements = 65536;

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

	static DWORD LUT[] = { //CRC32 LookUpTable
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
	0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
	0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
	0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
	0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
	0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
	0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
	0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
	0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
	0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
	0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
	0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
	0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
	0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
	0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
	0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
	0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
	0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
	0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
	0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
	0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
	0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4 };

	inline DWORD MapString(const wchar_t* Data) {

		DWORD Hash = 0;
		while (*Data)
		{
			TCHAR Ch = appToUpper(*Data++);
			WORD  B = Ch;
			Hash = (Hash >> 8) ^ LUT[(Hash ^ B) & 0xFF];
			B = Ch >> 8;
			Hash = (Hash >> 8) ^ LUT[(Hash ^ B) & 0xFF];
		}

		return Hash;
	}
	inline DWORD GetMappedIndex(DWORD Hash, size_t hasmapSize) {
		return ((((QWORD)Hash >> 16) ^ (INT)Hash) & (hasmapSize - 1));
	}

	inline DWORD Crc32(const wchar_t* Data) {
		DWORD Hash = 0;
		while (*Data)
		{
			WORD Ch = *Data++;
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

			DWORD Temp = (((Hash >> 8)) ^ GCRCTable[(Hash ^ Ch) & 0xFF]);
			Hash = (((Temp >> 8)) ^ GCRCTable[(Temp ^ (Ch >> 8)) & 0xFF]);
		}

		return Hash;
	}
	inline DWORD BucketIndex(const wchar_t* Data, size_t hashMapSize) {
		auto Hash = Crc32(Data);

		return (hashMapSize - 1) & ((Hash >> 16) ^ Hash);
	}

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
		DWORD Hash = 0;
		while (*Data)
		{
			WORD Ch = *Data++;
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

			Hash = (((Hash >> 8)) ^ GCRCTable[(Hash ^ Ch) & 0xFF]);
			Hash = (((Hash >> 8)) ^ GCRCTable[(Hash ^ (Ch >> 8)) & 0xFF]);
		}

		return Hash >> 0;
	}

	template<typename T>
	inline T ReverseBytes(T Value) {
		if constexpr (sizeof(T) == 1) {
		}
		else if constexpr (sizeof(T) == 2) {
			char* Bytes = (char*)&Value;

			std::swap(Bytes[0], Bytes[1]);
		}
		else if constexpr (sizeof(T) == 4) {

			char* Bytes = (char*)&Value;

			std::swap(Bytes[0], Bytes[3]);
			std::swap(Bytes[1], Bytes[2]);
		}
		else {

			char* Bytes = (char*)&Value;

			std::swap(Bytes[0], Bytes[7]);
			std::swap(Bytes[1], Bytes[6]);
			std::swap(Bytes[2], Bytes[4]);
		}

		return Value;
	}

	struct NOVTABLE DCSerializable {
		virtual bool Serialize(FIStream& Stream) = 0;
	};

	//DCArray
	template<typename T, size_t Size, size_t BasePadd = 0>
	struct DCArray : DCSerializable {
		T* Data = nullptr;
		UINT				Count = 0;
		UINT				Size = Size;

		DCArray() {}
		DCArray(DCArray&& other)noexcept {
			Data = other.Data;
			Count = other.Count;

			other.Data = nullptr;
			other.Count = 0;
		}
		DCArray& operator =(DCArray&& other) noexcept {
			if (&other == this) {
				return *this;
			}

			Data = other.Data;
			Count = other.Count;

			other.Data = nullptr;
			other.Count = 0;

			return *this;
		}

		~DCArray() {
			if (Data) {
				delete[] Data;
				Data = nullptr;
			}
		}

		//Cannot copy
		DCArray(const DCArray& other) = delete;
		DCArray& operator =(const DCArray& other) = delete;

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				UINT ToReadCount = Stream.ReadUInt32() - BasePadd;

				if (!CanFit(ToReadCount)) {
					return false;
				}

				for (size_t i = 0; i < ToReadCount; i++)
				{
					T& Object = NewItem();

					if (!Object.Serialize(Stream)) {
						continue;
					}
				}
			}
			else {
				Stream.WriteUInt32(Count + BasePadd);

				for (size_t i = 0; i < Count; i++)
				{
					if (!Data[i].Serialize(Stream)) {
						return false;
					}
				}
			}

			return true;
		}
		void Clear()noexcept {
			Count = 0;
		}

		T& operator[](INT Index)noexcept {
			return Data[Index];
		}
		inline T& NewItem()noexcept {
			if (!Data) {
				Data = new T[Size];
				if (!Data) {
					assert(false, "Failed to allocate DCArray Data");
				}
			}

			return Data[Count++];
		}
		inline bool CanFit(UINT Count) noexcept {
			if (this->Count + Count >= Size) {
				return false;
			}

			return true;
		}
	};

	template<typename T, size_t Size, size_t ElementSize>
	struct DCBlockArray : DCSerializable {
		T* Data = nullptr;
		UINT				UsedCount = 0;
		UINT				MaxCount = 0;

		DCBlockArray() : MaxCount(Size) {
			Data = new T[Size];
		}
		DCBlockArray(DCBlockArray&& other)noexcept {
			Data = other.Data;
			MaxCount = other.MaxCount;
			UsedCount = other.UsedCount;

			other.Data = nullptr;
			other.UsedCount = 0;
			other.MaxCount = 0;
		}
		DCBlockArray& operator =(DCBlockArray&& other) noexcept {
			if (&other == this) {
				return *this;
			}

			Data = other.Data;
			MaxCount = other.MaxCount;
			UsedCount = other.UsedCount;

			other.Data = nullptr;
			other.UsedCount = 0;
			other.MaxCount = 0;

			return *this;
		}

		~DCBlockArray() {
			if (Data) {
				delete[] Data;
				Data = nullptr;
			}
		}

		//Cannot copy
		DCBlockArray(const DCBlockArray& other) = delete;
		DCBlockArray& operator =(const DCBlockArray& other) = delete;

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				MaxCount = Stream.ReadUInt32();

				if (MaxCount > Size) {
					Message("DCBlockArray::Read new MaxCount value: %d", MaxCount);
					return false;
				}

				UINT ToReadUsedCount = Stream.ReadUInt32();

				for (size_t i = 0; i < ToReadUsedCount; i++)
				{
					T& Object = NewItem();

					if (!Object.Serialize(Stream)) {
						continue;
					}
				}
			}
			else {
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

		void Clear()noexcept {
			UsedCount = 0;
		}
		T& operator[](INT Index)noexcept {
			return Data[Index];
		}
		inline T& NewItem()noexcept {
			return Data[UsedCount++];
		}
		inline bool CanFit(UINT Count) noexcept {
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

	//Buckets
	template<typename T, size_t BucketsCount>
	struct DCBuckets : DCSerializable {
		DCArray<T, BucketsCount>					Buckets;

		DCBuckets() {}

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				for (size_t i = 0; i < BucketsCount; i++)
				{
					T& Bucket = Buckets.NewItem();

					if (!Bucket.Serialize(Stream)) {
						return false;
					}
				}
			}
			else {
				for (size_t i = 0; i < BucketsCount; i++)
				{
					if (!Buckets[i].Serialize(Stream)) {
						return false;
					}
				}
			}

			return true;
		}

		void Clear() noexcept {
			Buckets.Clear();
		}
	};

	template<size_t MaxCount>
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

		DCArray<BucketElement, MaxCount>		Elements;

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				const auto ToReadCount = Stream.ReadUInt32();

				for (size_t i = 0; i < ToReadCount; i++)
				{
					BucketElement& NewItem = Elements.NewItem();

					if (!NewItem.Serialize(Stream)) {
						return false;
					}
				}
			}
			else {
				Stream.WriteUInt32(Elements.Count);

				for (size_t i = 0; i < Elements.Count; i++)
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
			Elements = std::move(other.Elements);
		}
		StringsBucket& operator=(StringsBucket&& other) noexcept {
			if (&other == this) {
				return *this;
			}

			Elements = std::move(other.Elements);

			return *this;
		}

		//Cannot copy
		StringsBucket(const StringsBucket& other) = delete;
		StringsBucket& operator=(const StringsBucket& other) = delete;

		void Clear() noexcept {
			Elements.Clear();
		}
	};

	using DCStringsBuckets = DCBuckets<StringsBucket<CStringBucketSize>, CMaxStringBuckets>;

	template<size_t MaxStringBuckets, size_t MaxStringsCount = 65536, size_t MaxStringBlocks = CMaxStringBlocks>
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
			StringEntry& operator=(StringEntry&& other)noexcept {
				if (&other == this) {
					return *this;
				}

				Indices = other.Indices;
				CachedString = other.CachedString;

				return *this;
			}

			//Cannot copy
			StringEntry& operator=(const StringEntry& other) = delete;
			StringEntry(const StringEntry& other) = delete;

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

		DCArray<StringBlock, MaxStringBlocks>									StringBlocks;
		DCBuckets<StringsBucket<CStringBucketSize>, MaxStringBuckets>			Buckets;
		DCArray<StringEntry, MaxStringsCount, 1>								AllStrings;

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
			const wchar_t* Block = StringBlocks[BlockIndex].Block.Get();

			return &Block[StringIndex];
		}

		bool Prepare() noexcept {
			for (size_t i = 0; i < Buckets.Buckets.Count; i++) {
				auto& Bucket = Buckets.Buckets[i];
				for (size_t j = 0; j < Bucket.Elements.Count; j++) {
					Bucket.Elements[j].CachedString = GetString(Bucket.Elements[j].Indices.first, Bucket.Elements[j].Indices.second);
				}
			}

			for (size_t i = 0; i < AllStrings.Count; i++) {
				StringEntry& StringEntry = AllStrings[i];

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

			PresentStrings.clear();
			PresentStringsBig.clear();
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

			StringEntry& NewStringEntry = AllStrings.NewItem();

			StringId = (WORD)(AllStrings.Count - 1);

			NewStringEntry.Indices = Indices;
			NewStringEntry.CachedString = GetString(Indices.first, Indices.second);

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

			StringEntry& NewStringEntry = AllStrings.NewItem();

			//StringId = (WORD)(AllStrings.Data.size() - 1);

			NewStringEntry.Indices = OutIndices;
			NewStringEntry.CachedString = GetString(OutIndices.first, OutIndices.second);

			PresentStringsBig.insert({ String, OutIndices });

			return true;
		}

		bool BuildHashTables() noexcept {
			const DWORD Mask = Buckets.Buckets.Count - 1;
			DWORD Id = 1;

			for (size_t i = 0; i < AllStrings.Count; i++) {
				auto& StringEntry = AllStrings[i];

				const DWORD Hash = appStrCrc(StringEntry.CachedString.Get());
				const DWORD Index = (Hash >> 16 ^ Hash) & Mask;

				auto& HashmapElement = Buckets.Buckets[Index].Elements.NewItem();

				HashmapElement.CachedString = StringEntry.CachedString;
				HashmapElement.Indices = StringEntry.Indices;
				HashmapElement.Id = Id++;
				HashmapElement.Hash = Hash;
				HashmapElement.Length = wcslen(StringEntry.CachedString.Get()) + 1;
			}

			//sort all bucket elements by Hash
			for (size_t i = 0; i < Buckets.Buckets.Count; i++)
			{
				auto ElementsView =
					std::basic_string_view<StringsBucket<CStringBucketSize>::BucketElement>(Buckets.Buckets[i].Elements.Data, Buckets.Buckets[i].Elements.Count);

				std::sort(ElementsView.begin(), ElementsView.end(),
					[](const StringsBucket::BucketElement& a, const StringsBucket::BucketElement& b) -> bool
					{
						return a.Hash > b.Hash;
					});
			}

			return true;
		}

	private:
		bool InsertStringInBlock(const wchar_t* String, size_t StringSize, std::pair<WORD, WORD>& OutIndices)noexcept {
			for (size_t i = 0; i < StringBlocks.Count; i++) {
				if (StringBlocks[i].InsertString(String, StringSize, OutIndices.second)) {

					OutIndices.first = (WORD)(i);

					return true;
				}
			}

			StringBlock& NewBlock = StringBlocks.NewItem();

			if (!NewBlock.AllocateBlock()) {
				Message("StringBlock::Failed to allocate block!");
				return false;
			}

			OutIndices.first = (WORD)(StringBlocks.Count - 1);

			return NewBlock.InsertString(String, StringSize, OutIndices.second);
		}
	};

	struct DCIndex : DCSerializable {
		UINT16 Key1 = 0;
		UINT16 Key2 = 0;
		UINT16 Key3 = 0;
		UINT16 Key4 = 0;

		DCIndex() {}
		DCIndex(const DCIndex& other) {
			Key1 = other.Key1;
			Key2 = other.Key2;
			Key3 = other.Key3;
			Key4 = other.Key4;
		}
		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				Key1 = Stream.ReadUInt16();
				Key2 = Stream.ReadUInt16();
				Key3 = Stream.ReadUInt16();
				Key4 = Stream.ReadUInt16();
			}
			else {
				Stream.WriteUInt16(Key1);
				Stream.WriteUInt16(Key2);
				Stream.WriteUInt16(Key3);
				Stream.WriteUInt16(Key4);
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

			DCArray<Unk3Unk1_Item, 128>	Unk4;
			DCArray<Unk3Unk3Unk1_Item, 128>	Unk5;

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
			Unk3Unk3Unk1_Item& operator = (Unk3Unk3Unk1_Item&& other) noexcept {
				if (this == &other) {
					return *this;
				}

				Unk1 = other.Unk1;
				Unk2 = other.Unk2;
				Unk3 = other.Unk3;
				Unk4 = std::move(other.Unk4);
				Unk5 = std::move(other.Unk5);

				return *this;
			}
		};

		WORD								Unk1 = 0;
		WORD								Unk2 = 0;
		DWORD								Unk3 = 0;
		DCArray<Unk3Unk1_Item, 512>			Unk3Unk1;
		DCArray<Unk3Unk3Unk1_Item, 512>		Unk3Unk3;

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
		INT																								FormatVersion = 0;
		UINT64																							Unk1_8 = 0;
		INT																								Version = 0;

		UnkTree																							Unk;
		DCArray<DCIndex, CMaxIndices>																	Indices;
		DCArray<DCBlockArray<AttributeItem, CAttributesBlockSize, CAttributeSize>, CMaxAttributes>		Attributes;
		DCArray<DCBlockArray<ElementItem, CElementsBlockSize, CElementSize>, CMaxElements>				Elements;
		DCMap<1024>																						ValuesMap;
		DCMap<512>																						NamesMap;

		DWORD																							EndCount = 0;

		S1DataCenter() {
			/*	ValuesMap.Buckets.Count = 1024;
				NamesMap.Buckets.Count = 512;*/
		}

		bool PrepareForBuild()noexcept {
			/*for (size_t i = 0; i < ValuesMap.Buckets.Count; i++) {
				ValuesMap.Buckets.Buckets.push_back(StringsBucket());
			}

			for (size_t i = 0; i < NamesMap.Buckets.Count; i++) {
				NamesMap.Buckets.Buckets.push_back(StringsBucket());
			}*/

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

		bool InsertElementTree(ElementItemRaw* Root)noexcept {
			std::pair<WORD, WORD> LocalRootIndices;
			ElementItem* RootElement = AllocateElement(*Root, LocalRootIndices);
			if (!RootElement) {
				return false;
			}

			RootElement->ChildCount = (WORD)(Root->Children.size());

			if (Root->Children.size()) {
				ElementItem* RootChildrenStart = AllocateElements(Root->Children, RootElement->ChildrenIndices);
				if (!RootChildrenStart) {
					return false;
				}

				static std::atomic<const wchar_t*> asd;

				//Parse all sub tree
				for (size_t i = 0; i < Root->Children.size(); i++) {
					asd.store(Root->Children[i].Name.c_str());

					if (!InsertElementTreeImpl(&Root->Children[i], &RootChildrenStart[i])) {
						return false;
					}
				}
			}

			return true;
		}

	private:
		bool InsertElementTreeImpl(ElementItemRaw* RawElement, ElementItem* Element)noexcept {
			if (RawElement->Attributes.size()) {
				auto AttrStart = AllocateAttributes(RawElement->Attributes, Element->ArgsIndices);
				if (!AttrStart) {
					return false;
				}
			}

			Element->ArgsCount = (WORD)(RawElement->Attributes.size());
			Element->ChildCount = (WORD)(RawElement->Children.size());

			if (RawElement->Children.size()) {
				auto ChildStart = AllocateElements(RawElement->Children, Element->ChildrenIndices);
				if (!ChildStart) {
					return false;
				}

				for (size_t i = 0; i < RawElement->Children.size(); i++) {
					if (!InsertElementTreeImpl(&RawElement->Children[i], &ChildStart[i])) {
						return false;
					}
				}
			}

			return true;
		}

		ElementItem* AllocateElements(std::vector<ElementItemRaw>& RawElements, std::pair<WORD, WORD>& OutIndices)noexcept {
			auto& Block = GetElementsContainerForNewElements((INT)RawElements.size(), OutIndices.first);

			//start of this elements block
			OutIndices.second = (WORD)(Block.UsedCount);

			for (auto& RawElement : RawElements) {

				ElementItem& NewElement = Block.NewItem();

				//Prepare Element
				NewElement.Index = ++ElementsIndex;
				NewElement.LocationCache.first = OutIndices.first;
				NewElement.LocationCache.second = (WORD)(OutIndices.second - 1);
				NewElement.NameCache = std::move(RawElement.Name);

				WORD NameId;
				if (!NamesMap.InsertString(NewElement.NameCache.data(), NewElement.NameCache.size(), NameId)) {
					Message("AllocateElement::Failed to insert string [%ws]", NewElement.NameCache.size());
					return nullptr;
				}

				NewElement.Name = NameId;
			}

			return &Block[OutIndices.second];
		}
		AttributeItem* AllocateAttributes(std::vector<AttributeItemRaw>& RawAttributes, std::pair<WORD, WORD>& OutIndices)noexcept {
			auto& Block = GetAttributesContainerForNewAttributes((INT)RawAttributes.size(), OutIndices.first);

			//start of this attributes block
			OutIndices.second = (WORD)(Block.UsedCount);

			for (auto& RawAttribute : RawAttributes) {
				AttributeItem& NewAttribute = Block.NewItem();

				//Prepare Attribute
				NewAttribute.Index = ++AttributesIndex;
				NewAttribute.LocationCache.first = OutIndices.first;
				NewAttribute.LocationCache.second = (WORD)(Block.UsedCount - 1);
				NewAttribute.NameCache = std::move(RawAttribute.Name);
				NewAttribute.StringyfiedValue = std::move(RawAttribute.Value);
				NewAttribute.TypeInfo = RawAttribute.Type;


				if (RawAttribute.Type == AttributeType_String) {
					DWORD Crc32 = appStrCrcCaps(NewAttribute.NameCache.c_str());
					NewAttribute.TypeInfo = (WORD)(((Crc32 << 2) | AttributeType_String) & 0xffff);

					if (!ValuesMap.InsertString(NewAttribute.StringyfiedValue.data(), NewAttribute.StringyfiedValue.size(), NewAttribute.Indices)) {
						return nullptr;
					}

					//StringId not used for attribute value
				}

				WORD StringId;
				if (!NamesMap.InsertString(NewAttribute.NameCache.data(), NewAttribute.NameCache.size(), StringId)) {
					return nullptr;
				}

				NewAttribute.NameId = StringId;
			}

			return &Block[OutIndices.second];
		}
		ElementItem* AllocateElement(ElementItemRaw& RawElement, std::pair<WORD, WORD>& OutIndices) {
			auto& Block = GetElementsContainerForNewElements(1, OutIndices.first);

			//index of element
			OutIndices.second = (WORD)(Block.UsedCount);

			ElementItem& NewElement = Block.NewItem();

			//Prepare Element
			NewElement.Index = ++ElementsIndex;
			NewElement.LocationCache.first = OutIndices.first;
			NewElement.LocationCache.second = (WORD)(OutIndices.second - 1);
			NewElement.NameCache = std::move(RawElement.Name);

			WORD NameId;
			if (!NamesMap.InsertString(NewElement.NameCache.data(), NewElement.NameCache.size(), NameId)) {
				Message("AllocateElement::Failed to insert string [%ws]", NewElement.NameCache.size());
				return nullptr;
			}

			NewElement.Name = NameId;

			return &NewElement;
		}

		DCBlockArray<ElementItem, CElementsBlockSize, CElementSize>& GetElementsContainerForNewElements(INT Count, WORD& Out_Index)noexcept {
			//Try to find block that will contain all elements
			for (size_t i = 0; i < Elements.Count; i++)
			{
				if (Elements[i].CanFit(Count)) {

					Out_Index = (WORD)(i);

					return Elements[i];
				}
			}

			//Create new block array and reserve max
			DCBlockArray<ElementItem, CElementsBlockSize, CElementSize>& NewBlock = Elements.NewItem();

			Out_Index = (WORD)(Elements.Count - 1);

			return NewBlock;
		}
		DCBlockArray<AttributeItem, CAttributesBlockSize, CAttributeSize>& GetAttributesContainerForNewAttributes(INT Count, WORD& Out_Index)noexcept {
			//Try to find block that will contain all attributes
			for (size_t i = 0; i < Attributes.Count; i++)
			{
				if (Attributes[i].CanFit(Count)) {

					Out_Index = (WORD)(i);

					return Attributes[i];
				}
			}

			//Create new block array and reserve max
			DCBlockArray<AttributeItem, CAttributesBlockSize, CAttributeSize>& NewBlock = Attributes.NewItem();

			Out_Index = (WORD)(Elements.Count - 1);

			return NewBlock;

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