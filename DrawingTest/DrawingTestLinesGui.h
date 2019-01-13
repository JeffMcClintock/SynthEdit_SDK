#ifndef DRAWINGTESTLINESGUI_H_INCLUDED
#define DRAWINGTESTLINESGUI_H_INCLUDED

#include "Drawing.h"
#include "mp_sdk_gui2.h"
#include "../se_sdk3/TimerManager.h"

class DrawingTestLinesGui : public gmpi_gui::MpGuiGfxBase, public TimerClient
{
//	GmpiDrawing::Bitmap cachedRender_;
	GmpiDrawing::BitmapRenderTarget backbuffer;
	GmpiDrawing::Bitmap backbuffer2;

public:
	DrawingTestLinesGui();

	// overrides.
	virtual int32_t MP_STDCALL initialize() override;
	virtual int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext ) override;

	virtual bool OnTimer() override;

private:
 	void onSetTest();
 	void onSetFontface();
 	void onSetFontsize();
 	void onSetText();
 	void onSetGammaCorrection();
 	void onSetAdjust();
 	IntGuiPin pinTest;
 	IntGuiPin pinFontface;
 	IntGuiPin pinFontsize;
 	StringGuiPin pinText;
 	BoolGuiPin pinGammaCorrection;
 	FloatGuiPin pinAdjust;
};

#endif


