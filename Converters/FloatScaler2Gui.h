#ifndef FLOATSCALER2GUI_H_INCLUDED
#define FLOATSCALER2GUI_H_INCLUDED

#include "MP_SDK_GUI.h"

class FloatScaler2Gui : public MpGuiBase
{
public:
	FloatScaler2Gui( IMpUnknown* host );

private:
 	void onSetValueOut();
 	void onSetValueIn();

	FloatGuiPin valueOut;
 	FloatGuiPin multiplyby;
 	FloatGuiPin add;
 	FloatGuiPin valueIn;
};

#endif


