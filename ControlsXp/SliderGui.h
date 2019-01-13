#ifndef SLIDERGUI_H_INCLUDED
#define SLIDERGUI_H_INCLUDED

#include <algorithm>
#include "../se_sdk3/mp_sdk_gui2.h"
#include "../shared/ImageMetadata.h"
#include "BitmapWidget.h"
#include "EditWidget.h"
#include "TextWidget.h"

class SliderGui : public gmpi_gui::MpGuiGfxBase
{
	BitmapWidget bitmap;
	TextWidget headerWidget;
	EditWidget edit;
	Widget* captureWidget;

public:
	SliderGui();

	virtual int32_t MP_STDCALL initialize() override;

	virtual int32_t MP_STDCALL setHost(gmpi::IMpUnknown* host) override;
	virtual int32_t MP_STDCALL measure(GmpiDrawing_API::MP1_SIZE availableSize, GmpiDrawing_API::MP1_SIZE* returnDesiredSize) override;
	virtual int32_t MP_STDCALL arrange(GmpiDrawing_API::MP1_RECT finalRect) override;
	virtual int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext) override;

	virtual int32_t MP_STDCALL onPointerDown(int32_t flags, GmpiDrawing_API::MP1_POINT point) override;
	virtual int32_t MP_STDCALL onPointerMove(int32_t flags, GmpiDrawing_API::MP1_POINT point) override;
	virtual int32_t MP_STDCALL onPointerUp(int32_t flags, GmpiDrawing_API::MP1_POINT point) override;

	virtual int32_t MP_STDCALL populateContextMenu(float x, float y, gmpi::IMpUnknown* contextMenuItemsSink) override;
	virtual int32_t MP_STDCALL onContextMenu(int32_t selection) override;
	int32_t MP_STDCALL getToolTip(GmpiDrawing_API::MP1_POINT point, gmpi::IString* returnString) override
	{
		auto utf8String = (std::string)pinHint;
		returnString->setData(utf8String.data(), utf8String.size());

		return gmpi::MP_OK;
	}

private:
	void onSetValueIn();
	void onSetAppearance();
	void onSetTitle();
	void UpdateValuePinFromBitmap();
	void UpdateValuePinFromEdit();
	void UpdateEditText();

	FloatGuiPin pinValueIn;
	StringGuiPin pinItemList;
	StringGuiPin pinHint;
	StringGuiPin pinTitle;
	FloatGuiPin pinNormalised;

 	StringGuiPin pinNameIn;
 	StringGuiPin pinMenuItems;
 	IntGuiPin pinMenuSelection;
 	BoolGuiPin pinMouseDown;
 	FloatGuiPin pinRangeLo;
	FloatGuiPin pinRangeHi;
//	FloatGuiPin pinResetValue;
	BoolGuiPin pinShowReadout;
	IntGuiPin pinAppearance;
//	BoolGuiPin pinShowTitle;
};

#endif


