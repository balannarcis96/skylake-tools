#pragma once

namespace DCAdaptors {
	struct NOVTABLE DCAdaptor {

		virtual bool FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter) = 0;
		virtual TRef<S1DataCenter::S1DataCenter> ToDC() = 0;
	};

	struct DCValue {
		TRef<const wchar_t*>	Name = nullptr;
		TRef<const wchar_t*>	Value = nullptr;
	};

	struct DCJsonAdaptor : DCAdaptor {
		virtual bool FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter) override;
		virtual TRef<S1DataCenter::S1DataCenter> ToDC() override;
	};

	struct DCXMLAdaptor : DCAdaptor {
		struct XMLAttribute {
			std::wstring	Name;
			std::wstring	Value;
		};

		struct XMLNode {
			std::wstring				Name;
			std::vector<XMLAttribute>	Attributes;
			std::vector<XMLNode>		ChildNodes;

			XMLNode() {}
			XMLNode(const XMLNode& other) {}
			XMLNode(XMLNode&& other)noexcept{}
		};


		virtual bool FromDC(TRef<S1DataCenter::S1DataCenter> DataCenter) override;
		virtual TRef<S1DataCenter::S1DataCenter> ToDC() override;
	};
}

