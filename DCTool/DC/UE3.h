#pragma once

#define NO_BYTE_ORDER_SERIALIZE 0
#define PACKAGE_FILE_TAG			0x9E2A83C1
#define PACKAGE_FILE_TAG_SWAPPED	0xC1832A9E

// These macros are not safe to use unless data is UNSIGNED!
#define BYTESWAP_ORDER16_unsigned(x)   ((((x)>>8)&0xff)+ (((x)<<8)&0xff00))
#define BYTESWAP_ORDER32_unsigned(x)   (((x)>>24) + (((x)>>8)&0xff00) + (((x)<<8)&0xff0000) + ((x)<<24))

/** Compression Flag Masks **/
/** mask out compression type flags */
#define COMPRESSION_FLAGS_TYPE_MASK		0x0F
/** mask out compression type */
#define COMPRESSION_FLAGS_OPTIONS_MASK	0xF0

/**
 * Chunk size serialization code splits data into. The loading value CANNOT be changed without resaving all
 * compressed data which is why they are split into two separate defines.
 */
#define LOADING_COMPRESSION_CHUNK_SIZE_PRE_369  32768
#define LOADING_COMPRESSION_CHUNK_SIZE			131072
#define SAVING_COMPRESSION_CHUNK_SIZE			LOADING_COMPRESSION_CHUNK_SIZE

#define check assert

 // MS compilers support noop which discards everything inside the parens
#define debugf			__noop
#define warnf			__noop
#define KISMET_LOG		__noop
#define KISMET_WARN		__noop
#define KISMET_LOG_REF(obj)		__noop
#define KISMET_WARN_REF(obj)	__noop
#define verify __noop

static inline WORD BYTESWAP_ORDER16(WORD val)
{
#if PS3
	return __lhbrx(&val);
#else
	return(BYTESWAP_ORDER16_unsigned(val));
#endif
}

static inline SWORD BYTESWAP_ORDER16(SWORD val)
{
	WORD uval = *((WORD*)&val);
	uval = BYTESWAP_ORDER16_unsigned(uval);
	return(*((SWORD*)&uval));
}

static inline DWORD BYTESWAP_ORDER32(DWORD val)
{
#if PS3
	return __lwbrx(&val);
#else
	return(BYTESWAP_ORDER32_unsigned(val));
#endif
}

static inline INT BYTESWAP_ORDER32(INT val)
{
	DWORD uval = *((DWORD*)&val);
	uval = BYTESWAP_ORDER32_unsigned(uval);
	return(*((INT*)&uval));
}

static inline FLOAT BYTESWAP_ORDERF(FLOAT val)
{
	DWORD uval = *((DWORD*)&val);
	uval = BYTESWAP_ORDER32_unsigned(uval);
	return(*((FLOAT*)&uval));
}

static inline ULONGLONG BYTESWAP_ORDER64(ULONGLONG Value)
{
	ULONGLONG Swapped = 0;
	BYTE* Forward = (BYTE*)&Value;
	BYTE* Reverse = (BYTE*)&Swapped + 7;
	for (INT i = 0; i < 8; i++)
	{
		*(Reverse--) = *(Forward++); // copy into Swapped
	}
	return Swapped;
}

static inline void BYTESWAP_ORDER_TCHARARRAY(TCHAR* str)
{
	for (TCHAR* c = str; *c; ++c)
	{
		*c = BYTESWAP_ORDER16_unsigned(*c);
	}
}

//#define appMalloc _aligned_malloc
//#define appRealloc _aligned_realloc
//#define appFree _aligned_free

#define appMalloc malloc
#define appRealloc realloc
#define appFree free

/**
 * Flags controlling [de]compression
 */
enum ECompressionFlags
{
	/** No compression																*/
	COMPRESS_None = 0x00,
	/** Compress with ZLIB															*/
	COMPRESS_ZLIB = 0x01,
	/** Compress with LZO															*/
	COMPRESS_LZO = 0x02,
	/** Compress with LZX															*/
	COMPRESS_LZX = 0x04,
	/** Prefer compression that compresses smaller (ONLY VALID FOR COMPRESSION)		*/
	COMPRESS_BiasMemory = 0x10,
	/** Prefer compression that compresses faster (ONLY VALID FOR COMPRESSION)		*/
	COMPRESS_BiasSpeed = 0x20,
	/** If this flag is present, decompression will not happen on the SPUs.			*/
	COMPRESS_ForcePPUDecompressZLib = 0x80
};

/**
 * Helper structure for compression support, containing information on compressed
 * and uncompressed size of a chunk of data.
 */
struct FCompressedChunkInfo
{
	/** Compressed size of data.	*/
	INT	CompressedSize;
	/** Uncompresses size of data.	*/
	INT UncompressedSize;
};

/**
 * Thread-safe abstract compression routine. Compresses memory from uncompressed buffer and writes it to compressed
 * buffer. Updates CompressedSize with size of compressed data.
 *
 * @param	CompressedBuffer			Buffer compressed data is going to be written to
 * @param	CompressedSize	[in/out]	Size of CompressedBuffer, at exit will be size of compressed data
 * @param	UncompressedBuffer			Buffer containing uncompressed data
 * @param	UncompressedSize			Size of uncompressed data in bytes
 * @return TRUE if compression succeeds, FALSE if it fails because CompressedBuffer was too small or other reasons
 */
BOOL appCompressMemoryZLIB(void* CompressedBuffer, INT& CompressedSize, const void* UncompressedBuffer, INT UncompressedSize);

/**
 * Thread-safe abstract compression routine. Compresses memory from uncompressed buffer and writes it to compressed
 * buffer. Updates CompressedSize with size of compressed data.
 *
 * @param	UncompressedBuffer			Buffer containing uncompressed data
 * @param	UncompressedSize			Size of uncompressed data in bytes
 * @param	CompressedBuffer			Buffer compressed data is going to be read from
 * @param	CompressedSize				Size of CompressedBuffer data in bytes
 * @return TRUE if compression succeeds, FALSE if it fails because CompressedBuffer was too small or other reasons
 */
BOOL appUncompressMemoryZLIB(void* UncompressedBuffer, INT UncompressedSize, const void* CompressedBuffer, INT CompressedSize);

int appUncompressMemoryZLIB2(void* UncompressedBuffer, INT& UncompressedSize, const void* CompressedBuffer, INT CompressedSize);

/**
 * Thread-safe abstract compression routine. Compresses memory from uncompressed buffer and writes it to compressed
 * buffer. Updates CompressedSize with size of compressed data.
 *
 * @param	Flags						Flags to optionally control memory vs speed
 * @param	CompressedBuffer			Buffer compressed data is going to be written to
 * @param	CompressedSize	[in/out]	Size of CompressedBuffer, at exit will be size of compressed data
 * @param	UncompressedBuffer			Buffer containing uncompressed data
 * @param	UncompressedSize			Size of uncompressed data in bytes
 * @return TRUE if compression succeeds, FALSE if it fails because CompressedBuffer was too small or other reasons
 */
BOOL appCompressMemoryLZO(ECompressionFlags Flags, void* CompressedBuffer, INT& CompressedSize, const void* UncompressedBuffer, INT UncompressedSize);

/**
 * Thread-safe abstract compression routine. Compresses memory from uncompressed buffer and writes it to compressed
 * buffer. Updates CompressedSize with size of compressed data.
 *
 * @param	UncompressedBuffer			Buffer containing uncompressed data
 * @param	UncompressedSize			Size of uncompressed data in bytes
 * @param	CompressedBuffer			Buffer compressed data is going to be read from
 * @param	CompressedSize				Size of CompressedBuffer data in bytes
 * @return TRUE if compression succeeds, FALSE if it fails because CompressedBuffer was too small or other reasons
 */
BOOL appUncompressMemoryLZO(void* UncompressedBuffer, INT UncompressedSize, const void* CompressedBuffer, INT CompressedSize);

/**
 * Thread-safe abstract compression routine. Compresses memory from uncompressed buffer and writes it to compressed
 * buffer. Updates CompressedSize with size of compressed data. Compression controlled by the passed in flags.
 *
 * @param	Flags						Flags to control what method to use and optionally control memory vs speed
 * @param	CompressedBuffer			Buffer compressed data is going to be written to
 * @param	CompressedSize	[in/out]	Size of CompressedBuffer, at exit will be size of compressed data
 * @param	UncompressedBuffer			Buffer containing uncompressed data
 * @param	UncompressedSize			Size of uncompressed data in bytes
 * @return TRUE if compression succeeds, FALSE if it fails because CompressedBuffer was too small or other reasons
 */
BOOL appCompressMemory(ECompressionFlags Flags, void* CompressedBuffer, INT& CompressedSize, const void* UncompressedBuffer, INT UncompressedSize);

/**
 * Thread-safe abstract decompression routine. Uncompresses memory from compressed buffer and writes it to uncompressed
 * buffer. UncompressedSize is expected to be the exact size of the data after decompression.
 *
 * @param	Flags						Flags to control what method to use to decompress
 * @param	UncompressedBuffer			Buffer containing uncompressed data
 * @param	UncompressedSize			Size of uncompressed data in bytes
 * @param	CompressedBuffer			Buffer compressed data is going to be read from
 * @param	CompressedSize				Size of CompressedBuffer data in bytes
 * @param	bIsSourcePadded				Whether the source memory is padded with a full cache line at the end
 * @return TRUE if compression succeeds, FALSE if it fails because CompressedBuffer was too small or other reasons
 */
BOOL appUncompressMemory(ECompressionFlags Flags, void* UncompressedBuffer, INT UncompressedSize, const void* CompressedBuffer, INT CompressedSize, BOOL bIsSourcePadded = FALSE);

//
// Archive class. Used for loading, saving, and garbage collecting
// in a byte order neutral way.
//
class FArchive
{
public:
	// FArchive interface.
	virtual ~FArchive()
	{}
	virtual void Serialize(void* V, INT Length)
	{}
	virtual void SerializeBits(void* V, INT LengthBits)
	{
		Serialize(V, (LengthBits + 7) / 8);
		if (IsLoading())
		{
			((BYTE*)V)[LengthBits / 8] &= ((1 << (LengthBits & 7)) - 1);
		}
	}

	virtual void SerializeBits2(void* V, INT LengthBits)
	{
		if (ArForceByteSwapping) {
			INT BitCount = LengthBits - 1;

			if (LengthBits - 1 >= 0) {
				auto* bit = ((char*)V) + BitCount;
				do {
					SerializeBits(--bit, 1);

					--BitCount;
				} while (BitCount >= 0);
			}
		}
		else {
			SerializeBits(V, LengthBits);
		}
	}

	/**
	 * Serializes and compresses/ uncompresses data. This is a shared helper function for compression
	 * support. The data is saved in a way compatible with FIOSystem::LoadCompressedData.
	 *
	 * @param	V		Data pointer to serialize data from/ to
	 * @param	Length	Length of source data if we're saving, unused otherwise
	 * @param	Flags	Flags to control what method to use for [de]compression and optionally control memory vs speed when compressing
	 * @param	bTreatBufferAsFileReader TRUE if V is actually an FArchive, which is used when saving to read data - helps to avoid single huge allocations of source data
	 */
	void SerializeCompressed(void* V, INT Length, ECompressionFlags Flags, BOOL bTreatBufferAsFileReader = FALSE);

	void ThisContainsCode() { ArContainsCode = true; }	// Sets that this package contains code

	/**
	 * Indicate that this archive contains a ULevel or UWorld object.
	 */
	void ThisContainsMap()
	{
		ArContainsMap = true;
	}

	/**
	 * Indicates that this archive contains cooked data.
	 */
	void ThisContainsCookedData()
	{
		ArContainsCookedData = TRUE;
	}

	/**
	 * Indicate that this archive is currently serializing class/struct defaults
	 */
	void StartSerializingDefaults()
	{
		ArSerializingDefaults++;
	}

	/**
	 * Indicate that this archive is no longer serializing class/struct defaults
	 */
	void StopSerializingDefaults()
	{
		ArSerializingDefaults--;
	}

	/**
	 * Whether to allow serialization code to potentially remove references. Note that this
	 * is increasing/ decreasing an internal counter and hence supporting nesting so elimination
	 * might not be enabled after calling it.
	 *
	 * @param	Allow Whether to increase or decrease counter determining whether to allow elimination
	 * @return	TRUE if reference elimination is enabled, FALSE otherwise
	 */
	BOOL AllowEliminatingReferences(BOOL Allow)
	{
		ArAllowEliminatingReferences += Allow ? 1 : -1;
		return IsAllowingReferenceElimination();
	}

	// Hardcoded datatype routines that may not be overridden.
	FArchive& ByteOrderSerialize(void* V, INT Length)
	{
		BOOL SwapBytes = ArIsPersistent;
		if (SwapBytes)
		{
			// Transferring between memory and file, so flip the byte order.
			for (INT i = Length - 1; i >= 0; i--)
				Serialize((BYTE*)V + i, 1);
		}
		else
		{
			// Transferring around within memory, so keep the byte order.
			Serialize(V, Length);
		}
		return *this;
	}

	// Constructor.
	FArchive()
	{
		Reset();
	}

	virtual INT Tell()
	{
		return INDEX_NONE;
	}
	virtual INT TotalSize()
	{
		return INDEX_NONE;
	}
	virtual BOOL AtEnd()
	{
		INT Pos = Tell();
		return Pos != INDEX_NONE && Pos >= TotalSize();
	}
	virtual void Seek(INT InPos)
	{}

	// Status accessors.
	FORCEINLINE INT Ver()									const { return ArVer; }
	FORCEINLINE INT NetVer()								const { return ArNetVer & 0x7fffffff; }
	FORCEINLINE INT LicenseeVer()							const { return ArLicenseeVer; }
	FORCEINLINE BOOL IsLoading()							const { return ArIsLoading; }
	FORCEINLINE BOOL IsSaving()							const { return ArIsSaving; }
	FORCEINLINE BOOL IsSaveGame()							const { return ArIsSaveGame; }
	FORCEINLINE BOOL IsTransacting()						const { return ArIsTransacting; }
	FORCEINLINE BOOL WantBinaryPropertySerialization()		const { return ArWantBinaryPropertySerialization; }
	FORCEINLINE BOOL IsForcingUnicode()					const { return ArForceUnicode; }
	FORCEINLINE BOOL IsNet()								const { return (ArNetVer & 0x80000000) != 0; }
	FORCEINLINE BOOL IsPersistent()						const { return ArIsPersistent; }
	FORCEINLINE BOOL IsError()								const { return ArIsError; }
	FORCEINLINE BOOL IsCriticalError()						const { return ArIsCriticalError; }
	FORCEINLINE BOOL ContainsCookedData()					const { return ArContainsCookedData; }
	FORCEINLINE BOOL ForEdit()								const { return ArForEdit; }
	FORCEINLINE BOOL ForClient()							const { return ArForClient; }
	FORCEINLINE BOOL ForServer()							const { return ArForServer; }
	FORCEINLINE BOOL ContainsCode()						const { return ArContainsCode; }
	FORCEINLINE BOOL ContainsMap()							const { return ArContainsMap; }
	FORCEINLINE BOOL ForceByteSwapping()					const { return ArForceByteSwapping; }
	FORCEINLINE BOOL IsSerializingDefaults()				const { return ArSerializingDefaults > 0 ? TRUE : FALSE; }
	FORCEINLINE BOOL IsIgnoringArchetypeRef()				const { return ArIgnoreArchetypeRef; }
	FORCEINLINE BOOL IsIgnoringOuterRef()					const { return ArIgnoreOuterRef; }
	FORCEINLINE BOOL IsIgnoringClassRef()					const { return ArIgnoreClassRef; }
	FORCEINLINE BOOL IsAllowingReferenceElimination()		const { return ArAllowEliminatingReferences > 0 ? TRUE : FALSE; }
	FORCEINLINE BOOL IsAllowingLazyLoading()				const { return ArAllowLazyLoading; }
	FORCEINLINE BOOL IsObjectReferenceCollector()			const { return ArIsObjectReferenceCollector; }
	FORCEINLINE BOOL IsCountingMemory()					const { return ArIsCountingMemory; }
	FORCEINLINE DWORD GetPortFlags()						const { return ArPortFlags; }
	FORCEINLINE BOOL HasAnyPortFlags(DWORD Flags)		const { return (ArPortFlags & Flags) != 0; }
	FORCEINLINE BOOL HasAllPortFlags(DWORD Flags)		const { return (ArPortFlags & Flags) == Flags; }
	FORCEINLINE BOOL ShouldSkipBulkData()					const { return ArShouldSkipBulkData; }
	FORCEINLINE BOOL IsFinalPackageSave()					const { return ArIsFinalPackageSave; }
	FORCEINLINE INT GetMaxSerializeSize()					const { return ArMaxSerializeSize; }

	/**
	 * Sets the archive version number. Used by the code that makes sure that ULinkerLoad's internal
	 * archive versions match the file reader it creates.
	 *
	 * @param Ver	new version number
	 */
	void SetVer(INT InVer) { ArVer = InVer; }
	/**
	 * Sets the archive licensee version number. Used by the code that makes sure that ULinkerLoad's
	 * internal archive versions match the file reader it creates.
	 *
	 * @param Ver	new version number
	 */
	void SetLicenseeVer(INT InVer) { ArLicenseeVer = InVer; }

	/**
	 * Toggle saving as Unicode. This is needed when we need to make sure ANSI strings are saved as Unicode
	 *
	 * @param Enabled	set to true to force saving as Unicode
	 */
	void SetForceUnicode(BOOL Enabled) { ArForceUnicode = Enabled; }

	/**
	 * Toggle byte order swapping. This is needed in rare cases when we already know that the data
	 * swapping has already occured or if we know that it will be handled later.
	 *
	 * @param Enabled	set to true to enable byte order swapping
	 */
	void SetByteSwapping(BOOL Enabled) { ArForceByteSwapping = Enabled; }

	/**
	 * Sets the archive's property serialization modifier flags
	 *
	 * @param	InPortFlags		the new flags to use for property serialization
	 */
	void SetPortFlags(DWORD InPortFlags) { ArPortFlags = InPortFlags; }

	/**
	  * Sets the archive is a SaveGame
	  *
	  * @param  IsSaveGame		new flag
	  */
	void SetIsSaveGame(BOOL IsSaveGame) { ArIsSaveGame = IsSaveGame; }

	/**
	 * Returns if an async close operation has finished or not, as well as if there was an error
	 *
	 * @param bHasError TRUE if there was an error
	 *
	 * @return TRUE if the close operation is complete (or if Close is not an async operation)
	 */
	virtual BOOL IsCloseComplete(BOOL& bHasError) { bHasError = FALSE; return TRUE; }

	// Friend archivers.
	/*FORCEINLINE friend FArchive& operator<<(FArchive& Ar, ANSICHAR& C)
	{
		Ar.Serialize(&C, 1);
		return Ar;
	}*/
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, BYTE& B)
	{
		Ar.Serialize(&B, 1);
		return Ar;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, SBYTE& B)
	{
		Ar.Serialize(&B, 1);
		return Ar;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, WORD& W)
	{
		Ar.ByteOrderSerialize(&W, sizeof(W));
		return Ar;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, SWORD& S)
	{
		Ar.ByteOrderSerialize(&S, sizeof(S));
		return Ar;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, DWORD& D)
	{
		Ar.ByteOrderSerialize(&D, sizeof(D));
		return Ar;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, UINT& D)
	{
		Ar.ByteOrderSerialize(&D, sizeof(D));
		return Ar;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, INT& I)
	{
		Ar.ByteOrderSerialize(&I, sizeof(I));
		return Ar;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FLOAT& F)
	{
		Ar.ByteOrderSerialize(&F, sizeof(F));
		return Ar;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, DOUBLE& F)
	{
		Ar.ByteOrderSerialize(&F, sizeof(F));
		return Ar;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, ULONGLONG& Q)
	{
		Ar.ByteOrderSerialize(&Q, sizeof(Q));
		return Ar;
	}
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, LONGLONG& S)
	{
		Ar.ByteOrderSerialize(&S, sizeof(S));
		return Ar;
	}

protected:
	// Status variables.
	INT ArVer;
	INT ArNetVer;
	INT ArLicenseeVer;
	BOOL ArIsLoading;
	BOOL ArIsSaving;
	BOOL ArIsTransacting;
	/** Whether this archive wants properties to be serialized in binary form instead of tagged.			*/
	BOOL ArWantBinaryPropertySerialization;
	/** Whether this archive wants to always save strings in unicode format									*/
	BOOL ArForceUnicode;
	BOOL ArIsPersistent;
	BOOL ArForEdit;
	BOOL ArForClient;
	BOOL ArForServer;
	BOOL ArIsError;
	BOOL ArIsCriticalError;
	/** If TRUE, archive contains cooked data.																*/
	BOOL ArContainsCookedData;
	/** Quickly tell if an archive contains script code.													*/
	BOOL ArContainsCode;
	/** Used to determine whether FArchive contains a level or world.										*/
	BOOL ArContainsMap;
	/** Whether we should forcefully swap bytes.															*/
	BOOL ArForceByteSwapping;
	/** Whether we are currently serializing defaults. > 0 means yes, <= 0 means no							*/
	INT ArSerializingDefaults;
	/** If true, we will not serialize the ObjectArchetype reference in UObject.							*/
	BOOL ArIgnoreArchetypeRef;
	/** If true, we will not serialize the Outer reference in UObject.										*/
	BOOL ArIgnoreOuterRef;
	/** If true, UObject::Serialize will skip serialization of the Class property							*/
	BOOL ArIgnoreClassRef;
	/** Whether we are allowed to potentially NULL references to UObjects, > 0 means yes, <= 0 means no		*/
	INT ArAllowEliminatingReferences;
	/** Whether to allow lazy loading.																		*/
	BOOL ArAllowLazyLoading;
	/** Whether this archive only cares about serializing object references.								*/
	BOOL ArIsObjectReferenceCollector;
	/** Whether this archive is counting memory and therefore wants e.g. TMaps to be serialized.			*/
	BOOL ArIsCountingMemory;
	/** Modifier flags that be used when serializing UProperties											*/
	DWORD ArPortFlags;
	/** Whether bulk data serialization should be skipped or not.											*/
	BOOL ArShouldSkipBulkData;
	/** Set to indentity this archive as savegame file **/
	BOOL ArIsSaveGame;
	/**
	 * Indicates whether this is the final call to UObject::Serialize with ArIsSaving == TRUE during cooking.
	 * This is useful for knowing when to do slow tasks on save while cooking since UObject::Serialize is called multiple times on each object.
	 */
	BOOL ArIsFinalPackageSave;
	/** Max size of data that this archive is allowed to serialize */
	INT ArMaxSerializeSize;

	/**
	 * Resets all of the base archive members
	 */
	void Reset(void)
	{
		ArVer = 0;
		ArNetVer = 0;
		ArLicenseeVer = 0;
		ArIsLoading = FALSE;
		ArIsSaving = FALSE;
		ArIsTransacting = FALSE;
		ArWantBinaryPropertySerialization = FALSE;
		ArForceUnicode = FALSE;
		ArIsPersistent = FALSE;
		ArIsError = FALSE;
		ArIsCriticalError = FALSE;
		ArForEdit = TRUE;
		ArForClient = TRUE;
		ArForServer = TRUE;
		ArContainsCookedData = FALSE;
		ArContainsCode = FALSE;
		ArContainsMap = FALSE;
		ArForceByteSwapping = FALSE;
		ArSerializingDefaults = FALSE;
		ArIgnoreArchetypeRef = FALSE;
		ArIgnoreOuterRef = FALSE;
		ArIgnoreClassRef = FALSE;
		ArAllowEliminatingReferences = TRUE;
		ArAllowLazyLoading = FALSE;
		ArIsObjectReferenceCollector = FALSE;
		ArIsCountingMemory = FALSE;
		ArPortFlags = 0;
		ArShouldSkipBulkData = FALSE;
		ArIsSaveGame = FALSE;
		ArIsFinalPackageSave = FALSE;
		ArMaxSerializeSize = 0;
	}
};

/**
 * Similar to FMemoryReader, but able to internally
 * manage the memory for the buffer.
 */
class FBufferReader : public FArchive
{
public:
	/**
	 * Constructor
	 *
	 * @param Data Buffer to use as the source data to read from
	 * @param Size Size of Data
	 * @param bInFreeOnClose If TRUE, Data will be appFree'd when this archive is closed
	 * @param bIsPersistent Uses this value for ArIsPersistent
	 * @param bInSHAVerifyOnClose It TRUE, an async SHA verification will be done on the Data buffer (bInFreeOnClose will be passed on to the async task)
	 */
	FBufferReader(void* Data, INT Size, BOOL bInFreeOnClose, BOOL bIsPersistent = FALSE)
		: ReaderData(Data)
		, ReaderPos(0)
		, ReaderSize(Size)
		, bFreeOnClose(bInFreeOnClose)
	{
		ArIsLoading = TRUE;
		ArIsPersistent = bIsPersistent;
	}

	~FBufferReader()
	{
		Close();
	}
	BOOL Close()
	{
		if (bFreeOnClose)
		{
			appFree(ReaderData);
			ReaderData = NULL;
		}
		return !ArIsError;
	}
	void Serialize(void* Data, INT Num)
	{
		check(ReaderPos >= 0);
		check(ReaderPos + Num <= ReaderSize);
		appMemcpy(Data, (BYTE*)ReaderData + ReaderPos, Num);
		ReaderPos += Num;
	}
	INT Tell()
	{
		return ReaderPos;
	}
	INT TotalSize()
	{
		return ReaderSize;
	}
	void Seek(INT InPos)
	{
		check(InPos >= 0);
		check(InPos <= ReaderSize);
		ReaderPos = InPos;
	}
	BOOL AtEnd()
	{
		return ReaderPos >= ReaderSize;
	}

	static FBufferReader* FromFile(const char* fileName);

	void SaveToFile(const char* file);

	UINT64	BitMap = 0;

protected:
	void* ReaderData;
	INT		ReaderPos;
	INT		ReaderSize;
	BOOL	bFreeOnClose;


};

#define CRC32_POLY 0x04c11db7

#define UPPER_LOWER_DIFF	32
inline TCHAR appToUpper(TCHAR c)
{
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	//@hack - ideally, this would be data driven or use some sort of lookup table
	// some special cases
	switch (UNICHAR(c))
	{
		// these special chars are not 32 apart
	case 255: return 159; // diaeresis y
	case 156: return 140; // digraph ae

	// characters within the 192 - 255 range which have no uppercase/lowercase equivalents
	case 240:
	case 208:
	case 223:
	case 247:
		return c;
	}

	if ((c >= TEXT('a') && c <= TEXT('z')) || (c > 223 && c < 255))
	{
		return c - UPPER_LOWER_DIFF;
	}

	// no uppercase equivalent
	return c;
}

extern DWORD GCRCTable[256];

/** Case insensitive string hash function. */
inline DWORD appStrihash(const TCHAR* Data)
{
	DWORD Hash = 0;
	while (*Data)
	{
		TCHAR Ch = appToUpper(*Data++);
		WORD  B = Ch;
		Hash = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
		B = Ch >> 8;
		Hash = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
	}
	return Hash;
}

/** Case sensitive string hash function. */
inline DWORD appStrihashEx(const TCHAR* Data)
{
	DWORD Hash = 0;
	while (*Data)
	{
		TCHAR Ch = *Data++;
		WORD  B = Ch;
		Hash = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
		B = Ch >> 8;
		Hash = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
	}
	return Hash;
}

/** Case insensitive string hash function. */
inline DWORD appStrihash(const ANSICHAR* Data)
{
	DWORD Hash = 0;
	while (*Data)
	{
		TCHAR Ch = appToUpper(*Data++);
		WORD  B = Ch;
		Hash = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
	}
	return Hash;
}

inline DWORD dcStrihash(const TCHAR* Data, size_t hashMapSize) {
	auto Hash = appStrihash(Data);

	return (Hash >> 16 ^ Hash) & (hashMapSize - 1);
}

inline uint8_t* appHexDecode(const char* in, size_t len, uint8_t* out)
{
	unsigned int i, t, hn, ln;

	for (t = 0, i = 0; i < len; i += 2, ++t) {

		hn = in[i] > '9' ? (in[i] | 32) - 'a' + 10 : in[i] - '0';
		ln = in[i + 1] > '9' ? (in[i + 1] | 32) - 'a' + 10 : in[i + 1] - '0';

		out[t] = (hn << 4) | ln;
	}

	return out;
}