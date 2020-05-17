#include "../App.h"

bool DCAdaptors::DCJsonAdaptor::FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter)
{
	return false;
}

TRef<S1DataCenter::S1DataCenter> DCAdaptors::DCJsonAdaptor::ToDC()
{
	return TRef<S1DataCenter::S1DataCenter>();
}

bool DCAdaptors::DCXMLAdaptor::FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter)
{
	return false;
}

TRef<S1DataCenter::S1DataCenter> DCAdaptors::DCXMLAdaptor::ToDC()
{
	return TRef<S1DataCenter::S1DataCenter>();
}
