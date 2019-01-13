#ifndef DRAWINGTESTGUI_H_INCLUDED
#define DRAWINGTESTGUI_H_INCLUDED

#include "../se_sdk3/mp_sdk_gui2.h"

class DrawingTestGui : public gmpi_gui::MpGuiGfxBase
{
	IntGuiPin pinTestType;
	IntGuiPin pinFontface;
	IntGuiPin pinFontsize;
	StringGuiPin pinText;
	BoolGuiPin pinApplyAlphaCorrection;
	FloatGuiPin pinAdjust;
	void MyApplyGammaCorrection(GmpiDrawing::Bitmap& bitmap);

public:
	DrawingTestGui();
	void refresh();
	void drawGammaTest(GmpiDrawing::Graphics& g);

	// overrides.
	virtual int32_t MP_STDCALL measure(GmpiDrawing_API::MP1_SIZE availableSize, GmpiDrawing_API::MP1_SIZE* returnDesiredSize) override;
	void drawGradient(GmpiDrawing::Graphics & g);
	virtual int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext) override;
};

#endif


