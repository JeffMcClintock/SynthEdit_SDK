#pragma once

#include <chrono>
#include "../se_sdk3/mp_sdk_gui2.h"
#include "../shared/FontCache.h"

class FreqAnalyserGui :
	public gmpi_gui::MpGuiGfxBase, public FontCacheClient
{
	GmpiDrawing::Bitmap cachedBackground_;
	float GraphXAxisYcoord;
	float currentBackgroundSampleRate;
	GmpiDrawing::PathGeometry geometry;
	GmpiDrawing::PathGeometry lineGeometry;

public:
	FreqAnalyserGui();

	// IMpGraphics overrides.
	virtual int32_t MP_STDCALL measure(GmpiDrawing_API::MP1_SIZE availableSize, GmpiDrawing_API::MP1_SIZE* returnDesiredSize) override;
	virtual int32_t MP_STDCALL arrange(GmpiDrawing_API::MP1_RECT finalRect) override;
	virtual int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext) override;
	
	void onModeChanged();
	void onValueChanged();

	BlobGuiPin pinSpectrum;
	IntGuiPin pinMode;
	IntGuiPin pinDbHigh;
	IntGuiPin pinDbLow;
};
