#include "mp_sdk_gui2.h"
#include "Drawing.h"

using namespace gmpi;
using namespace GmpiDrawing;

class MonitorGui : public gmpi_gui::MpGuiGfxBase
{

public:
	MonitorGui()
	{
	}

	int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext ) override
	{
		Graphics g(drawingContext);

		auto textFormat = GetGraphicsFactory().CreateTextFormat();
		auto brush = g.CreateSolidColorBrush(Color::Red);

		g.DrawTextU("Hello World!", textFormat, 0.0f, 0.0f, brush);

		return gmpi::MP_OK;
	}
};

namespace
{
	auto r = Register<MonitorGui>::withId(L"SE Monitor");
}
