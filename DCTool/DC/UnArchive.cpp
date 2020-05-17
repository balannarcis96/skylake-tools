#include "../App.h"

/*-----------------------------------------------------------------------------
	FArchive implementation.
-----------------------------------------------------------------------------*/

/**
 * FCompressedChunkInfo serialize operator.
 */
FArchive& operator<<(FArchive& Ar, FCompressedChunkInfo& Chunk)
{
	// The order of serialization needs to be identical to the memory layout as the async IO code is reading it in bulk.
	// The size of the structure also needs to match what is being serialized.
	Ar << Chunk.CompressedSize;
	Ar << Chunk.UncompressedSize;
	return Ar;
}

// Helper structure to keep information about async chunks that are in-flight.
class FAsyncCompressionChunk
{
public:
	/** Pointer to source (uncompressed) memory.								*/
	void* UncompressedBuffer;
	/** Pointer to destination (compressed) memory.								*/
	void* CompressedBuffer;
	/** Compressed size in bytes as passed to/ returned from compressor.		*/
	INT CompressedSize;
	/** Uncompressed size in bytes as passed to compressor.						*/
	INT UncompressedSize;
	/** Flags to control compression											*/
	ECompressionFlags Flags;

	/**
	 * Constructor, zeros everything
	 */
	FAsyncCompressionChunk()
		: UncompressedBuffer(0)
		, CompressedBuffer(0)
		, CompressedSize(0)
		, UncompressedSize(0)
		, Flags(ECompressionFlags(0))
	{
	}

	/** Give the name for external event viewers
	* @return	the name to display in external event viewers
	*/
	static const TCHAR* Name()
	{
		return TEXT("FAsyncCompressionChunks");
	}
};

/**
 * Serializes and compresses/ uncompresses data. This is a shared helper function for compression
 * support. The data is saved in a way compatible with FIOSystem::LoadCompressedData.
 *
 * @note: the way this code works needs to be in line with FIOSystem::LoadCompressedData implementations
 * @note: the way this code works needs to be in line with FAsyncIOSystemBase::FulfillCompressedRead
 *
 * @param	V		Data pointer to serialize data from/to, or a FileReader if bTreatBufferAsFileReader is TRUE
 * @param	Length	Length of source data if we're saving, unused otherwise
 * @param	Flags	Flags to control what method to use for [de]compression and optionally control memory vs speed when compressing
 * @param	bTreatBufferAsFileReader TRUE if V is actually an FArchive, which is used when saving to read data - helps to avoid single huge allocations of source data
 */
void FArchive::SerializeCompressed(void* V, INT Length, ECompressionFlags Flags, BOOL bTreatBufferAsFileReader)
{
	if (IsLoading())
	{
		// Serialize package file tag used to determine endianess.
		FCompressedChunkInfo PackageFileTag;
		PackageFileTag.CompressedSize = 0;
		PackageFileTag.UncompressedSize = 0;
		*this << PackageFileTag;
		BOOL bWasByteSwapped = PackageFileTag.CompressedSize != PACKAGE_FILE_TAG;

		// Read in base summary.
		FCompressedChunkInfo Summary;
		*this << Summary;

		if (bWasByteSwapped)
		{
			Summary.CompressedSize = BYTESWAP_ORDER32(Summary.CompressedSize);
			Summary.UncompressedSize = BYTESWAP_ORDER32(Summary.UncompressedSize);
			PackageFileTag.UncompressedSize = BYTESWAP_ORDER32(PackageFileTag.UncompressedSize);
		}
		else
		{
			assert(PackageFileTag.CompressedSize == PACKAGE_FILE_TAG);
		}

		// Handle change in compression chunk size in backward compatible way.
		INT LoadingCompressionChunkSize = PackageFileTag.UncompressedSize;
		if (LoadingCompressionChunkSize == PACKAGE_FILE_TAG)
		{
			LoadingCompressionChunkSize = LOADING_COMPRESSION_CHUNK_SIZE;
		}

		// Figure out how many chunks there are going to be based on uncompressed size and compression chunk size.
		INT	TotalChunkCount = (Summary.UncompressedSize + LoadingCompressionChunkSize - 1) / LoadingCompressionChunkSize;

		// Allocate compression chunk infos and serialize them, keeping track of max size of compression chunks used.
		FCompressedChunkInfo* CompressionChunks = new FCompressedChunkInfo[TotalChunkCount];
		INT						MaxCompressedSize = 0;
		for (INT ChunkIndex = 0; ChunkIndex < TotalChunkCount; ChunkIndex++)
		{
			*this << CompressionChunks[ChunkIndex];
			if (bWasByteSwapped)
			{
				CompressionChunks[ChunkIndex].CompressedSize = BYTESWAP_ORDER32(CompressionChunks[ChunkIndex].CompressedSize);
				CompressionChunks[ChunkIndex].UncompressedSize = BYTESWAP_ORDER32(CompressionChunks[ChunkIndex].UncompressedSize);
			}
			MaxCompressedSize = Max(CompressionChunks[ChunkIndex].CompressedSize, MaxCompressedSize);
		}

		INT Padding = 0;

		// Set up destination pointer and allocate memory for compressed chunk[s] (one at a time).
		BYTE* Dest = (BYTE*)V;
		void* CompressedBuffer = appMalloc(MaxCompressedSize + Padding);

		// Iterate over all chunks, serialize them into memory and decompress them directly into the destination pointer
		for (INT ChunkIndex = 0; ChunkIndex < TotalChunkCount; ChunkIndex++)
		{
			const FCompressedChunkInfo& Chunk = CompressionChunks[ChunkIndex];
			// Read compressed data.
			Serialize(CompressedBuffer, Chunk.CompressedSize);
			// Decompress into dest pointer directly.
			verify(appUncompressMemory(Flags, Dest, Chunk.UncompressedSize, CompressedBuffer, Chunk.CompressedSize, (Padding > 0) ? TRUE : FALSE));
			// And advance it by read amount.
			Dest += Chunk.UncompressedSize;
		}

		// Free up allocated memory.
		appFree(CompressedBuffer);
		delete[] CompressionChunks;
	}
	else if (IsSaving())
	{
		check(Length > 0);

		// Serialize package file tag used to determine endianess in LoadCompressedData.
		FCompressedChunkInfo PackageFileTag;
		PackageFileTag.CompressedSize = PACKAGE_FILE_TAG;
		PackageFileTag.UncompressedSize = GSavingCompressionChunkSize;
		*this << PackageFileTag;

		// Figure out how many chunks there are going to be based on uncompressed size and compression chunk size.
		INT	TotalChunkCount = (Length + GSavingCompressionChunkSize - 1) / GSavingCompressionChunkSize + 1;

		// Keep track of current position so we can later seek back and overwrite stub compression chunk infos.
		INT StartPosition = Tell();

		// Allocate compression chunk infos and serialize them so we can later overwrite the data.
		FCompressedChunkInfo* CompressionChunks = new FCompressedChunkInfo[TotalChunkCount];
		for (INT ChunkIndex = 0; ChunkIndex < TotalChunkCount; ChunkIndex++)
		{
			*this << CompressionChunks[ChunkIndex];
		}

		// The uncompressd size is equal to the passed in length.
		CompressionChunks[0].UncompressedSize = Length;
		// Zero initialize compressed size so we can update it during chunk compression.
		CompressionChunks[0].CompressedSize = 0;

		// Set up source pointer amount of data to copy (in bytes)
		BYTE* Src;
		// allocate memory to read into
		if (bTreatBufferAsFileReader)
		{
			Src = (BYTE*)appMalloc(GSavingCompressionChunkSize);
			check(((FArchive*)V)->IsLoading());
		}
		else
		{
			Src = (BYTE*)V;
		}
		INT		BytesRemaining = Length;
		// Start at index 1 as first chunk info is summary.
		INT		CurrentChunkIndex = 1;
		// 2 times the uncompressed size should be more than enough; the compressed data shouldn't be that much larger
		INT		CompressedBufferSize = 2 * GSavingCompressionChunkSize;
		void* CompressedBuffer = appMalloc(CompressedBufferSize);

		while (BytesRemaining > 0)
		{
			INT BytesToCompress = Min(BytesRemaining, GSavingCompressionChunkSize);
			INT CompressedSize = CompressedBufferSize;

			// read in the next chunk from the reader
			if (bTreatBufferAsFileReader)
			{
				((FArchive*)V)->Serialize(Src, BytesToCompress);
			}

			verify(appCompressMemory(Flags, CompressedBuffer, CompressedSize, Src, BytesToCompress));
			// move to next chunk if not reading from file
			if (!bTreatBufferAsFileReader)
			{
				Src += BytesToCompress;
			}

			Serialize(CompressedBuffer, CompressedSize);
			// Keep track of total compressed size, stored in first chunk.
			CompressionChunks[0].CompressedSize += CompressedSize;

			// Update current chunk.
			check(CurrentChunkIndex < TotalChunkCount);
			CompressionChunks[CurrentChunkIndex].CompressedSize = CompressedSize;
			CompressionChunks[CurrentChunkIndex].UncompressedSize = BytesToCompress;
			CurrentChunkIndex++;

			BytesRemaining -= GSavingCompressionChunkSize;
		}

		// free the buffer we read into
		if (bTreatBufferAsFileReader)
		{
			appFree(Src);
		}

		// Free allocated memory.
		appFree(CompressedBuffer);

		// Overrwrite chunk infos by seeking to the beginning, serializing the data and then
		// seeking back to the end.
		INT EndPosition = Tell();
		// Seek to the beginning.
		Seek(StartPosition);
		// Serialize chunk infos.
		for (INT ChunkIndex = 0; ChunkIndex < TotalChunkCount; ChunkIndex++)
		{
			*this << CompressionChunks[ChunkIndex];
		}
		// Seek back to end.
		Seek(EndPosition);

		// Free intermediate data.
		delete[] CompressionChunks;
	}
}
