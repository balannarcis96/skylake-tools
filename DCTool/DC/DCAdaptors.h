#pragma once

namespace DCAdaptors {
	struct NOVTABLE DCAdaptor {
		TRef<S1DataCenter::S1DataCenter>		DataCenter = nullptr;
		std::vector<std::string>				AllXmlFiles;

		virtual bool FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter) = 0;
		virtual bool Export(const wchar_t* DirName) = 0;

		virtual bool BuildDC(const wchar_t* DirName, TRef<S1DataCenter::S1DataCenter> DataCenter) = 0;

		virtual bool SerializeMetadata(const wchar_t* RootDir, INT FilesCount)noexcept;
		virtual bool ReadMetadata(const wchar_t* File, INT& FilesCount)noexcept;

		bool FindAllFilesInDirectory(const char* DCDir, const char * extension, std::vector<std::string>& outFiles) const noexcept;
	};

	struct DCJsonAdaptor : DCAdaptor {
		virtual bool FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter) override;
		virtual bool Export(const wchar_t* DirName) override;
		virtual bool BuildDC(const wchar_t* DirName, TRef<S1DataCenter::S1DataCenter> DataCenter) { return false; }
	};

	struct DCXMLAdaptor : DCAdaptor {
		virtual bool FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter) override;
		virtual bool Export(const wchar_t* DirName) override;
		virtual bool BuildDC(const wchar_t* DirName, TRef<S1DataCenter::S1DataCenter> DataCenter);

	private:
		bool ParseXmlFile(const char* FileName, char* Buffer, size_t BufferLength, ElementItemRaw* Parent) noexcept;
	};
}

