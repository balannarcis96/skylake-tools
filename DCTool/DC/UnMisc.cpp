#include "../App.h"

#include <zlib.h>

#include <lzo/lzoconf.h>
#include <lzo/lzopro/lzo1x.h>
#include <lzo/lzopro/lzo1y.h>
#include <lzo/lzo1f.h>

/**
 * Callback memory allocation function for the LZO*99* compressor to use
 *
 * @param UserData	Points to the GLZOCallbacks structure
 * @param Items		Number of "items" to allocate
 * @param Size		Size of each "item" to allocate
 *
 * @return A pointer to a block of memory Items * Size big
 */
lzo_voidp __LZO_CDECL LZOMalloc(lzo_callback_p UserData, lzo_uint Items, lzo_uint Size)
{
	return appMalloc(Items * Size);
}

/**
 * Callback memory deallocation function for the LZO*99* compressor to use
 *
 * @param UserData	Points to the GLZOCallbacks structure
 * @param Ptr		Pointer to memory to free
 */
void __LZO_CDECL LZOFree(lzo_callback_p UserData, lzo_voidp Ptr)
{
	appFree(Ptr);
}

lzo_callback_t GLZOCallbacks =
{
	LZOMalloc, // allocation routine
	LZOFree, // deallocation routine
	0, // progress callback
	NULL, // user pointer
	0, // user data
	0 // user data
};

// NOTE: The following will be all cleaned up when we decide on the final choice of compressors to use!
// each compression method needs a different amount of work mem
#define LZO_STANDARD_MEM		LZOPRO_LZO1X_1_14_MEM_COMPRESS
#define LZO_SPEED_MEM			LZOPRO_LZO1X_1_08_MEM_COMPRESS
#define LZO_SIZE_MEM			0

// allocate to fit the biggest one
#define LZO_WORK_MEM_SIZE		(Max<INT>(Max<INT>(LZO_STANDARD_MEM, LZO_SPEED_MEM), LZO_SIZE_MEM))

// each compression function must be in the same family (lzo1x, etc) for decompression
#define LZO_STANDARD_COMPRESS	lzopro_lzo1x_1_14_compress
#define LZO_SPEED_COMPRESS		lzopro_lzo1x_1_08_compress
#define LZO_SIZE_COMPRESS(in, in_len, out, out_len) lzopro_lzo1x_99_compress(in, in_len, out, out_len, &GLZOCallbacks, 10);
#if __WIN32__
#define LZO_DECOMPRESS			lzopro_lzo1x_decompress_safe
#else
#define LZO_DECOMPRESS			lzopro_lzo1x_decompress
#endif

void InitializeLZO()
{
	static BOOL bIsInitialized = FALSE;
	if (!bIsInitialized)
	{
		// Initialized lzo library and verify it succeeds.
		if (lzo_init() != LZO_E_OK) {
			Message("Failed to init lzo");
		};
	}
}

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
BOOL appCompressMemoryZLIB(void* CompressedBuffer, INT& CompressedSize, const void* UncompressedBuffer, INT UncompressedSize)
{
	// Zlib wants to use unsigned long.
	unsigned long ZCompressedSize = CompressedSize;
	unsigned long ZUncompressedSize = UncompressedSize;
	// Compress data
	BOOL bOperationSucceeded = compress((BYTE*)CompressedBuffer, &ZCompressedSize, (const BYTE*)UncompressedBuffer, ZUncompressedSize) == Z_OK ? TRUE : FALSE;
	// Propagate compressed size from intermediate variable back into out variable.
	CompressedSize = ZCompressedSize;
	return bOperationSucceeded;
}

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
BOOL appUncompressMemoryZLIB(void* UncompressedBuffer, INT UncompressedSize, const void* CompressedBuffer, INT CompressedSize)
{
	// Zlib wants to use unsigned long.
	unsigned long ZCompressedSize = CompressedSize;
	unsigned long ZUncompressedSize = UncompressedSize;

	// Uncompress data.
	BOOL bOperationSucceeded = uncompress((BYTE*)UncompressedBuffer, &ZUncompressedSize, (const BYTE*)CompressedBuffer, ZCompressedSize) == Z_OK ? TRUE : FALSE;

	// Sanity check to make sure we uncompressed as much data as we expected to.
	check(UncompressedSize == ZUncompressedSize);
	return bOperationSucceeded;
}

int appUncompressMemoryZLIB2(void* UncompressedBuffer, INT& UncompressedSize, const void* CompressedBuffer, INT CompressedSize)
{
	z_stream stream;

	stream.next_in = (Bytef*)CompressedBuffer;
	stream.avail_in = CompressedSize;
	stream.next_out = (Bytef*)UncompressedBuffer;
	stream.avail_out = UncompressedSize;
	stream.zalloc = nullptr;
	stream.zfree = nullptr;

	auto result = inflateInit_(&stream, "1.2.3", sizeof(stream));
	if (!result) {

		result = inflate(&stream, Z_FINISH);
		if (result == 1) {
			UncompressedSize = stream.total_out;
			result = inflateEnd(&stream);
		}
		else {
			inflateEnd(&stream);

			if (result != Z_NEED_DICT && (result != Z_BUF_ERROR || stream.avail_in)) {
				return result;
			}

			return Z_DATA_ERROR;
		}
	}

	return result;
}

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
BOOL appCompressMemoryLZO(ECompressionFlags Flags, void* CompressedBuffer, INT& CompressedSize, const void* UncompressedBuffer, INT UncompressedSize)
{
	void* LZOWorkMemoryBase = NULL;

	// we use a temporary buffer here that is bigger than the uncompressed size. this
	// is because LZO expects CompressedSize to be bigger than UncompressedSize, in
	// case the data is incompressible. Since there is no guarantee that CompressedSize
	// is big enough, we can't use it, because LZO will happily overwrite past the end
	// of CompressedBuffer (since it doesn't take as input the size of CompressedBuffer)

	const PTRINT LZOAlignment = 0x10000;  // LZO uses the pointer low bits as a seed, we want to avoid that

	BYTE* CompressScratchBufferBase = (BYTE*)appMalloc(UncompressedSize + LZO_WORK_MEM_SIZE + LZOAlignment);
	BYTE* CompressScratchBuffer = Align(CompressScratchBufferBase, LZOAlignment);

	// LZO reads past the end of the source data, so we need to ensure that data is always the same.
	BYTE* SourceDataBase = (BYTE*)appMalloc(UncompressedSize + LZO_WORK_MEM_SIZE + LZOAlignment);
	BYTE* SourceData = Align(SourceDataBase, LZOAlignment);
	appMemzero(SourceData, UncompressedSize + LZO_WORK_MEM_SIZE);
	appMemcpy(SourceData, UncompressedBuffer, UncompressedSize);

	// out variable for how big the compressed data actually is
	lzo_uint FinalCompressedSize;
	// attempt to compress the data 
	INT Result = LZO_E_OK;

	// Make sure LZO is initialized before calling it.
	InitializeLZO();

	if (Flags & COMPRESS_BiasSpeed)
	{
		// Zero initialize scratch memory. The LZO compressor makes decisions based on values of uninitialized data.
		LZOWorkMemoryBase = appMalloc(LZO_SPEED_MEM + LZOAlignment);
		BYTE* LZOWorkMemory = (BYTE*)Align(LZOWorkMemoryBase, LZOAlignment);
		appMemzero(LZOWorkMemory, LZO_SPEED_MEM);

		Result = LZO_SPEED_COMPRESS(SourceData, UncompressedSize, CompressScratchBuffer, &FinalCompressedSize, LZOWorkMemory);
	}
	else if (Flags & COMPRESS_BiasMemory)
	{
		Result = LZO_SIZE_COMPRESS(SourceData, UncompressedSize, CompressScratchBuffer, &FinalCompressedSize);
	}
	else
	{
		// Zero initialize scratch memory. The LZO compressor makes decisions based on values of uninitialized data.
		LZOWorkMemoryBase = appMalloc(LZO_STANDARD_MEM + LZOAlignment);
		BYTE* LZOWorkMemory = (BYTE*)Align(LZOWorkMemoryBase, LZOAlignment);
		appMemzero(LZOWorkMemory, LZO_STANDARD_MEM);

		Result = LZO_STANDARD_COMPRESS(SourceData, UncompressedSize, CompressScratchBuffer, &FinalCompressedSize, LZOWorkMemory);
	}

	// this shouldn't ever fail, apparently unless something catastrophic happened
	// but the docs are really not clear, because there are no docs
	check(Result == LZO_E_OK);

	// by default we succeeded (ie fit into available memory)
	BOOL Return = TRUE;

	// if the compressed size will fit in the CompressedBuffer, copy it in
	if (FinalCompressedSize <= (DWORD)CompressedSize)
	{
		// copy the data
		appMemcpy(CompressedBuffer, CompressScratchBuffer, FinalCompressedSize);
	}
	else
	{
		// if it doesn't fit, then this function has failed
		Return = FALSE;
	}

	// Free up the work memory
	if (LZOWorkMemoryBase)
	{
		appFree(LZOWorkMemoryBase);
	}

	appFree(SourceDataBase);
	appFree(CompressScratchBufferBase);

	// if this compression succeeded or failed, return how big it compressed it to
	// this way, on a failure, it can be called again with a big enough buffer
	CompressedSize = FinalCompressedSize;

	// return our success/failure
	return(Return);
}

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
BOOL appUncompressMemoryLZO(void* UncompressedBuffer, INT UncompressedSize, const void* CompressedBuffer, INT CompressedSize)
{
	// Make sure LZO is initialized before calling it.
	InitializeLZO();

	// LZO wants unsigned. Initialized to uncompressed size as safe version uses this for bounds checking.
	lzo_uint FinalUncompressedSize = UncompressedSize;
	INT Result = LZO_DECOMPRESS((const BYTE*)CompressedBuffer, CompressedSize, (BYTE*)UncompressedBuffer, &FinalUncompressedSize, NULL);

	// if the call failed, return FALSE
	if (Result != LZO_E_OK)
	{
		return FALSE;
	}

	// success
	return TRUE;
}

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
BOOL appCompressMemory(ECompressionFlags Flags, void* CompressedBuffer, INT& CompressedSize, const void* UncompressedBuffer, INT UncompressedSize)
{
	// make sure a valid compression scheme was provided
	check(Flags & (COMPRESS_ZLIB | COMPRESS_LZO | COMPRESS_LZX));

	BOOL bCompressSucceeded = FALSE;

	// Always bias for speed if option is set.
	if (GAlwaysBiasCompressionForSize)
	{
		INT NewFlags = Flags;
		NewFlags &= ~COMPRESS_BiasSpeed;
		NewFlags |= COMPRESS_BiasMemory;
		Flags = (ECompressionFlags)NewFlags;
	}

	switch (Flags & COMPRESSION_FLAGS_TYPE_MASK)
	{
	case COMPRESS_ZLIB:
		bCompressSucceeded = appCompressMemoryZLIB(CompressedBuffer, CompressedSize, UncompressedBuffer, UncompressedSize);
		break;
	case COMPRESS_LZO:
		bCompressSucceeded = appCompressMemoryLZO(Flags, CompressedBuffer, CompressedSize, UncompressedBuffer, UncompressedSize);
		break;
	default:
		warnf(TEXT("appCompressMemory - This compression type not supported"));
		bCompressSucceeded = FALSE;
	}

	return bCompressSucceeded;
}

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
BOOL appUncompressMemory(ECompressionFlags Flags, void* UncompressedBuffer, INT UncompressedSize, const void* CompressedBuffer, INT CompressedSize, BOOL bIsSourcePadded /*= FALSE*/)
{
	// make sure a valid compression scheme was provided
	check(Flags & (COMPRESS_ZLIB | COMPRESS_LZO | COMPRESS_LZX));

	BOOL bUncompressSucceeded = FALSE;

	switch (Flags & COMPRESSION_FLAGS_TYPE_MASK)
	{
	case COMPRESS_ZLIB:
		bUncompressSucceeded = appUncompressMemoryZLIB2(UncompressedBuffer, UncompressedSize, CompressedBuffer, CompressedSize);
		break;
	case COMPRESS_LZO:
		bUncompressSucceeded = appUncompressMemoryLZO(UncompressedBuffer, UncompressedSize, CompressedBuffer, CompressedSize);
		break;
	default:
		warnf(TEXT("appUncompressMemory - This compression type not supported"));
		bUncompressSucceeded = FALSE;
	}

	return bUncompressSucceeded;
	}
