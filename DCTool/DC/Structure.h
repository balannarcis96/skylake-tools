#pragma once

#define DC_64 1

namespace S1DataCenter {

	enum AttributeType : WORD {
		AttributeType_None = 0,
		AttributeType_1 = 1 << 0,
		AttributeType_2 = 1 << 1,
		AttributeType_3 = 1 << 2,
		AttributeType_4 = 1 << 3,
		AttributeType_5 = 1 << 4,
		AttributeType_6 = 1 << 5,
		AttributeType_7 = 1 << 6,
		AttributeType_8 = 1 << 7,
		AttributeType_9 = 1 << 8,
		AttributeType_10 = 1 << 9,
		AttributeType_11 = 1 << 10,
		AttributeType_12 = 1 << 11,
		AttributeType_13 = 1 << 12,
		AttributeType_14 = 1 << 13,
		AttributeType_15 = 1 << 14,
		AttributeType_16 = 1 << 15,

		AttributeType_String = AttributeType_1 | AttributeType_2
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

		QWORD Hash = 0;
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
	template<typename T, bool isPrimitive = false, bool hasMaxCount = false>
	struct DCArray : DCSerializable {
		typedef void(*CallbackPtr)(void*, T*);

		std::vector<T>		Data;
		UINT32				MaxCount = 0;
		INT					MinCount = -1;
		TRef<void>			Parent = nullptr;
		CallbackPtr			Callback = nullptr;

		DCArray() {}
		DCArray(DCArray&& other)noexcept {
			Data = std::move(other.Data);
			MaxCount = other.MaxCount;
			MinCount = other.MinCount;
			Parent = other.Parent;
			Callback = other.Callback;

			other.MaxCount = 0;
		}

		virtual bool Serialize(FIStream& Stream) override {
			INT StartI = MinCount < 0 ? 0 : MinCount;

			if (Stream.IsLoading()) {
				Data.clear();

				UINT32 Count = Stream.ReadUInt32();

				if constexpr (hasMaxCount) {
					MaxCount = Stream.ReadUInt32();
				}
				else {
					MaxCount = 0;
				}

				//if ((INT)Count > MinCount) {
				Data.reserve(Count);

				for (size_t i = StartI; i < Count; i++)
				{
					auto OldPos = Stream._pos;

					if constexpr (isPrimitive) {
						T Value = ReadScalar(Stream);

						Data.push_back(T);
					}
					else {
						T Object;

						if (Callback) {
							Callback(Parent.Get(), &Object);
						}

						if (!Object.Serialize(Stream)) {
							Message("Error while serializing DCArray Item at position %d [isLoading=%d]", OldPos, (int)Stream.IsLoading());
							return false;
						}

						Data.push_back(std::move(Object));
					}
				}
				//}
			}
			else {
				Stream.WriteUInt32((UINT32)Data.size() + StartI);

				if constexpr (hasMaxCount) {
					Stream.WriteUInt32(MaxCount);
				}

				//Serialize unread data 
				for (size_t i = 0; i < StartI; i++)
				{
					auto OldPos = Stream._pos;

					if constexpr (isPrimitive) {
						WriteScalar(Stream, T());
					}
					else {
						T Default;

						if (!Default.Serialize(Stream)) {
							Message("Error while serializing DCArray Item at position %d [isLoading=%d]", OldPos, (int)Stream.IsLoading());
							return false;
						}
					}
				}

				//if ((INT)Data.size() > MinCount) {
				for (size_t i = StartI; i < Data.size(); i++)
				{
					auto OldPos = Stream._pos;

					if constexpr (isPrimitive) {
						WriteScalar(Stream, Data[i]);
					}
					else {
						if (!Data[i].Serialize(Stream)) {
							Message("Error while serializing DCArray Item at position %d [isLoading=%d]", OldPos, (int)Stream.IsLoading());
							return false;
						}
					}
				}
				//}
			}

			return true;
		}
		void operator =(DCArray& other) {
			Data = std::move(other.Data);
			MaxCount = other.MaxCount;
			MinCount = other.MinCount;
			Parent = other.Parent;
			Callback = other.Callback;

			other.MaxCount = 0;
		}

		T& operator[](INT I) noexcept {
			return Data[I];
		}

	private:
		T ReadScalar(FIStream& Stream) {
			char Value[sizeof(T)];

			Stream.Read((UINT8*)Value, sizeof(T));

			return *((T*)Value);
		}
		T WriteScalar(FIStream& Stream, T Value) {
			Stream.Write((UINT8*)&Value, sizeof(T));
		}
	};

	template<typename T>
	struct DCPrimitiveArray : DCArray<T, true> {};

	template<typename T, bool isPrimitive = false>
	struct MaxCountDCArray : DCArray<T, isPrimitive, true> {};

	//String Blocks
	struct StringBlock : DCSerializable {
		TRef<wchar_t>	Block = nullptr;
		INT				BlockSize = 0;
		INT				MaxCount = 0;

		StringBlock() {}
		StringBlock(StringBlock&& other)noexcept {
			Block = other.Block.Release();
			BlockSize = other.BlockSize;
			MaxCount = other.MaxCount;
		}

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				BlockSize = Stream.ReadInt32();
				MaxCount = Stream.ReadInt32();

				Block = new wchar_t[BlockSize];

				Stream.Read((UINT8*)Block.Get(), BlockSize * 2);
			}
			else {
				Stream.WriteInt32(BlockSize);
				Stream.WriteInt32(MaxCount);

				Stream.Write((UINT8*)Block.Get(), BlockSize * 2);
			}

			return true;
		};
		~StringBlock() {
			if (Block.Get()) {
				delete[] Block.Get();
				Block = nullptr;
			}
		}
	};

	typedef DCArray<StringBlock> DCStringBlocks;

	//Buckets
	template<typename T>
	struct DCBuckets : DCSerializable {
		DWORD										Count = 0;
		TRef<T>										Buckets = nullptr;

		TRef<DCStringBlocks>						Blocks = nullptr;

		virtual bool Serialize(FIStream& Stream) override {
			if (!Count) {
				return true;
			}
			if (Stream.IsLoading()) {
				std::unique_ptr<T[]> TempBuckets = nullptr;

				TempBuckets = std::unique_ptr<T[]>(new T[Count]);
				for (size_t i = 0; i < Count; i++)
				{
					TempBuckets[i].Blocks = Blocks;

					if (!TempBuckets[i].Serialize(Stream)) {
						return false;
					}
				}

				Buckets = TempBuckets.release();
			}
			else {
				for (size_t i = 0; i < Count; i++)
				{
					if (!(Buckets.Get())[i].Serialize(Stream)) {
						return false;
					}
				}
			}

			return true;
		}

		inline const T& GetBucket(const wchar_t* string) const noexcept {
			const auto Hash = MapString(string);
			const auto Index = GetMappedIndex(Hash, Count);

			return (Buckets.Get())[Index];
		}
		inline const wchar_t* GetElement(const wchar_t* string) const noexcept {
			const auto Hash = Crc32(string);
			const auto Index = GetMappedIndex(Hash, Count);

			return (Buckets.Get())[Index].GetElement(Hash);
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

			return (Buckets.Get())[Index];
		}

		inline const TRef<T> GetBucket(INT Index)const noexcept {
			return Buckets.Get() ? &(Buckets.Get())[Index] : nullptr;
		}
		inline const TRef<StringBlock> GetStringBlock(INT Index)const noexcept {
			return Blocks.Get() ? &Blocks->Data[Index] : nullptr;
		}

		~DCBuckets() {
			if (Buckets.Get())
			{
				delete[] Buckets.Get();
				Buckets = nullptr;
			}
		}
	};

	struct StringsBucket : DCSerializable {
		struct BucketElement : DCSerializable {
			DWORD							Hash;
			DWORD							Length;
			DWORD							Id;
			std::pair<WORD, WORD>			Indices;

			TRef<const wchar_t>				CachedString = nullptr;

			TRef<DCStringBlocks>			Blocks = nullptr;

			BucketElement() {
				Hash = 0;
				Length = 0;
				Id = 0;
				Indices = { 0,0 };
			}

			virtual bool Serialize(FIStream& Stream) override {
				if (Stream.IsLoading()) {
					Hash = Stream.ReadUInt32();
					Length = Stream.ReadUInt32();
					Id = Stream.ReadUInt32();
					Indices.first = Stream.ReadUInt16();
					Indices.second = Stream.ReadUInt16();

					auto& BlocksRef = *(Blocks.Get());

					const auto Block = BlocksRef[Indices.first].Block.Get();

					CachedString = &Block[Indices.second];
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
		TRef<BucketElement>							Elements = nullptr;

		//Ref
		TRef<DCStringBlocks>						Blocks = nullptr;

		const wchar_t* GetElement(const DWORD Hash) const noexcept {
			if (!Count) {
				return nullptr;
			}

			for (size_t i = 0; i < Count; i++)
			{
				if (Elements.Get()[i].Hash == Hash) {
					return Elements.Get()[i].CachedString.Get();
				}
			}

			return nullptr;
		}

		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				Count = Stream.ReadUInt32();

				if (Count > 1) {
					Elements = new BucketElement[Count];
				}
			}
			else {
				Stream.WriteUInt32(Count);
			}

			if (Count) {

				for (size_t i = 0; i < Count; i++)
				{
					(Elements.Get())[i].Blocks = Blocks;

					if (!(Elements.Get())[i].Serialize(Stream)) {
						return false;
					}
				}
			}

			return true;
		}
	};

	struct DCStringsBuckets : DCBuckets<StringsBucket> {};

	struct DCMap : DCSerializable {
		struct StringEntry : DCSerializable {
			std::pair<WORD, WORD>			Indices = { 0,0 };
			TRef<const wchar_t>				CachedString = nullptr;

			//Ref
			TRef<DCStringBlocks>			Blocks = nullptr;

			StringEntry() {}
			StringEntry(StringEntry&& other)noexcept {
				Indices = other.Indices;
				CachedString = other.CachedString;
				Blocks = other.Blocks;
			}
			virtual bool Serialize(FIStream& Stream) override {
				if (Stream.IsLoading()) {
					Indices.first = Stream.ReadUInt16();
					Indices.second = Stream.ReadUInt16();

					if (*((INT*)&Indices) == -1) {
						CachedString = nullptr;
					}
					else {
						auto& BlocksRef = *(Blocks.Get());
						const auto Block = BlocksRef[Indices.first].Block.Get();

						CachedString = &Block[Indices.second];
					}
				}
				else {
					Stream.WriteUInt16(Indices.first);
					Stream.WriteUInt16(Indices.second);
				}

				return true;
			}
		};

		DCStringBlocks			StringBlocks;
		DCStringsBuckets		Buckets;
		DCArray<StringEntry>	AllStrings;

		DCMap() {
			Buckets.Blocks = &StringBlocks;

			AllStrings.Parent = this;
			AllStrings.Callback = (DCArray<StringEntry>::CallbackPtr)PrepareItem;
			AllStrings.MinCount = 1;
		}

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
			return &(StringBlocks[BlockIndex].Block.Get())[StringIndex];
		}

	private:
		static void PrepareItem(DCMap* This, StringEntry* Item) {
			Item->Blocks = &This->StringBlocks;
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

		bool IsValid() const noexcept {
			return (TypeInfo & 3) != 0;
		}
		bool IsInt() const noexcept {
			return (TypeInfo & 3) == 1;
		}
		bool IsFloat() const noexcept {
			return (TypeInfo & 3) == 2;
		}
		bool IsString() const noexcept {
			return (TypeInfo & 3) == 3;
		}

#if DC_64
		DWORD					Padding = 0;
#endif

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

		}
		virtual bool Serialize(FIStream& Stream) override {
			if (Stream.IsLoading()) {
				NameId = Stream.ReadUInt16();
				TypeInfo = (AttributeType)Stream.ReadUInt16();

				Indices.first = Stream.ReadUInt16();
				Indices.second = Stream.ReadUInt16();

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
		}

		void MessageThis() const noexcept {
			if (!IsValid()) {
				return;
			}

			if (IsInt() && NameRef.Get()) {
				Message("ARG %ws=%d", NameRef.Get(), IntValue);
			}
			else if (IsFloat() && NameRef.Get()) {
				Message("ARG %ws=%f", NameRef.Get(), FloatValue);

			}
			else if (StringRef.Get() && NameRef.Get()) {
				Message("ARG %ws=%ws", NameRef.Get(), StringRef.Get());
			}
		}
	};

	struct ElementItem : DCSerializable {
		UINT16						Name = 0;
		UINT16						Index = 0;
		UINT16						ArgsCount = 0;
		UINT16						ChildCount = 0;
		std::pair<WORD, WORD>		ArgsIndices = { 0,0 };
		std::pair<WORD, WORD>		ChildrenIndices = { 0,0 };

#if DC_64
		DWORD						Padd1 = 0;
		DWORD						Padd2 = 0;
#endif

		TRef<const wchar_t>			NameRef = nullptr;
		TRef<const AttributeItem>	Arguments = nullptr;
		TRef<const ElementItem>		Children = nullptr;

		ElementItem() {}
		ElementItem(const ElementItem& other) {
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

			Arguments = other.Arguments;
			Children = other.Children;
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

			Arguments = other.Arguments;
			Children = other.Children;
		}

		void MessageThis() {
			if (!Arguments.Get() && !NameRef.Get()) {
				return;
			}

			if (NameRef.Get() && !Arguments.Get()) {
				Message("Element Name=%ws Value is null", NameRef.Get());
			}
			else if (NameRef.Get() && Arguments.Get()) {
				if (Arguments->IsInt()) {
					Message("Element Name=%ws Value=%d", NameRef.Get(), Arguments->IntValue);
				}
				else if (Arguments->IsFloat()) {
					Message("Element Name=%ws Value=%f", NameRef.Get(), Arguments->FloatValue);
				}
				else if (Arguments->StringRef.Get()) {
					Message("Element Name=%ws Value=%ws", NameRef.Get(), Arguments->StringRef.Get());
				}
				else {
					Message("Element Name=%ws String is null", NameRef.Get());
				}
			}
			else if(Arguments.Get()){
				if (Arguments->IsInt()) {
					Message("Element Value=%d", Arguments->IntValue);
				}
				else if (Arguments->IsFloat()) {
					Message("Element Value=%f", Arguments->FloatValue);
				}
				else if (Arguments->StringRef.Get()) {
					Message("Element Value=%ws", Arguments->StringRef.Get());
				}
				else {
					Message("Invalid Element");
				}
			}
		}
	};

	struct S1DataCenter : DCSerializable {
		INT											FormatVersion = 0;
		UINT64										Unk1_8 = 0;
		INT											Version = 0;

		UnkTree										Unk;
		DCArray<DCIndex>							Indices;
		DCArray<MaxCountDCArray<AttributeItem>>		Attributes;
		DCArray<MaxCountDCArray<ElementItem>>		Elements;
		DCMap										ValuesMap;
		DCMap										NamesMap;

		DWORD										EndCount = 0;

		S1DataCenter() {
			ValuesMap.Buckets.Count = 1024;
			NamesMap.Buckets.Count = 512;
		}

		virtual bool Serialize(FIStream& Stream) override;

		bool Prepare();

	private:
		bool PrepareAttributes();
		bool PrepareElements();
	};
}