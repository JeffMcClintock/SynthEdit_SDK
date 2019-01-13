#ifndef SPRING2GUI_H_INCLUDED
#define SPRING2GUI_H_INCLUDED

#include "MP_SDK_GUI.h"

class Spring2Gui : public MpGuiBase
{
public:
	Spring2Gui(IMpUnknown* host);

private:
	void onSetMouseDown();

	void onEnabled();

	bool prevMouseDown; // only for backward compatible version, prevents spurios signal on startup (may be crashing things).

	FloatGuiPin normalisedValue;
	FloatGuiPin resetValue;
	BoolGuiPin mouseDown;
	BoolGuiPin enable;
};

#endif


