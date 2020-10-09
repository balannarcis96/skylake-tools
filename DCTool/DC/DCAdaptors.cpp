#include "../App.h"

bool DCAdaptors::DCJsonAdaptor::FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter)
{
	return false;
}

TRef<S1DataCenter::S1DataCenter> DCAdaptors::DCJsonAdaptor::ToDC()
{
	return TRef<S1DataCenter::S1DataCenter>();
}

bool DCAdaptors::DCJsonAdaptor::Export(const char* FileName)
{
	return true;
}

bool DCAdaptors::DCXMLAdaptor::FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter)
{
	IsFromDC = TRUE;
	this->DataCenter = DataCenter;
	auto RootElement = DataCenter->GetRootElement();

	return true;
}

TRef<S1DataCenter::S1DataCenter> DCAdaptors::DCXMLAdaptor::ToDC()
{
	IsFromDC = FALSE;

	return TRef<S1DataCenter::S1DataCenter>();
}

bool DCAdaptors::DCXMLAdaptor::Export(const char* RootDir)
{
	FIStream MetaStream;
	MetaStream._isLoading = false;

	if (!DataCenter->Unk.Serialize(MetaStream)) {
		return false;
	}

	std::string MetaFilePath = RootDir;
	DataCenter->BuildDCExportName(MetaFilePath);
	MetaFilePath += "\\DC_Metadata.bin";
	if (!MetaStream.save_to_file(MetaFilePath.c_str())) {
		Message("Failed to Save %s file", MetaFilePath.c_str());
	}

	return true;
}
