#pragma once
#include "Drawing.h"
#include "mp_sdk_gui2.h"

class Widget
{
protected:
	GmpiDrawing::Rect position;
	bool dirty;
	gmpi_gui::IMpGraphicsHost* guiHost_;
	gmpi::IMpUserInterfaceHost2* patchMemoryHost_;

public:
	Widget();
	virtual ~Widget();

	void setPosition(GmpiDrawing::Rect pPosition)
	{
		position = pPosition;
	}
	
	const GmpiDrawing::Rect& getRect()
	{
		return position;
	}

	bool ClearDirty()
	{
		auto r = dirty;
		dirty = false;
		return r;
	}

	bool widgetHitTest(GmpiDrawing::Point point)
	{
		return point.x >= position.left && point.x < position.right && point.y >= position.top && point.y < position.bottom;
	}

	gmpi::IMpUserInterfaceHost2* getHost(){ return patchMemoryHost_; };
	gmpi_gui::IMpGraphicsHost* getGuiHost(){ return guiHost_; };
	virtual int32_t setHost(gmpi::IMpUnknown* host)
	{
		host->queryInterface(gmpi_gui::SE_IID_GRAPHICS_HOST, reinterpret_cast<void**>( &guiHost_));
		host->queryInterface(gmpi::MP_IID_UI_HOST2, reinterpret_cast<void**>( &patchMemoryHost_ ));
		return gmpi::MP_OK;
	}

	virtual GmpiDrawing::Size getSize() = 0;
	virtual void OnRender(GmpiDrawing::Graphics& dc) = 0;

	virtual bool onPointerDown(int32_t flags, GmpiDrawing_API::MP1_POINT point){ return false; };
	virtual void onPointerMove(int32_t flags, GmpiDrawing_API::MP1_POINT point){};
	virtual void onPointerUp(int32_t flags, GmpiDrawing_API::MP1_POINT point){};
};

