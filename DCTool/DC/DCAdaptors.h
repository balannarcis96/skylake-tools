#pragma once

namespace DCAdaptors {
	struct NOVTABLE DCAdaptor {

		virtual bool FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter) = 0;
		virtual TRef<S1DataCenter::S1DataCenter> ToDC() = 0;
		virtual bool Export(const char* DirName) = 0;
	};

	struct DCJsonAdaptor : DCAdaptor {
		virtual bool FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter) override;
		virtual TRef<S1DataCenter::S1DataCenter> ToDC() override;
		virtual bool Export(const char* DirName) override;
	};

	struct DCXMLAdaptor : DCAdaptor {
		using xmldoc = rapidxml::xml_document<>;

		virtual bool FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter) override;
		virtual TRef<S1DataCenter::S1DataCenter> ToDC() override;
		virtual bool Export(const char* DirName) override;

	private:
		BOOL									IsFromDC = FALSE;
		std::vector<xmldoc*>					Documents;
		TRef<S1DataCenter::S1DataCenter>		DataCenter = nullptr;
	};
}

