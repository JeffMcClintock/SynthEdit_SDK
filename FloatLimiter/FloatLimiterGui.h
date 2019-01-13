#ifndef FLOATLIMITERGUI_H_INCLUDED
#define FLOATLIMITERGUI_H_INCLUDED

#include "MP_SDK_GUI.h"
#include "TimerManager.h"

class FloatLimiterGui : public MpGuiBase, public TimerClient
{
public:
	FloatLimiterGui( IMpUnknown* host );
	virtual bool OnTimer();

private:
 	void onSetValue();
 	FloatGuiPin min;
 	FloatGuiPin max;
 	FloatGuiPin value;
};

#endif


