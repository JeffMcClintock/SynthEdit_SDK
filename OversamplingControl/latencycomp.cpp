#include "mp_sdk_gui2.h"

using namespace gmpi;
using namespace gmpi_gui;

/*
class LatencyCompGui : public MpGuiInvisibleBase
{
public:
	LatencyCompGui()
	{
		initializePin(pinHostMaxLatencyCompensation, &LatencyCompGui::onSetHostLatency);
		initializePin(pinMaxLatencyCompensation, &LatencyCompGui::onSetLatency);
//		initializePin(pinListItems);
	}

	virtual int32_t MP_STDCALL initialize() override
	{
		return MpGuiInvisibleBase::initialize();
//		pinListItems = L"Off,On";
	}

	void onSetHostLatency()
	{
		pinMaxLatencyCompensation = pinHostMaxLatencyCompensation;
	}

	void onSetLatency()
	{
		pinHostMaxLatencyCompensation = pinMaxLatencyCompensation;
	}

private:
	IntGuiPin pinHostMaxLatencyCompensation;
	IntGuiPin pinMaxLatencyCompensation;
	StringGuiPin pinListItems;
};

GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, LatencyCompGui, L"SE Latency Compensation");
*/
