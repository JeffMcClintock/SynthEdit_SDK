#ifndef SYSTEMCOMMANDGUI_H_INCLUDED
#define SYSTEMCOMMANDGUI_H_INCLUDED

#include "MP_SDK_GUI.h"

class SystemCommandGui : public MpGuiBase
{
public:
	SystemCommandGui( IMpUnknown* host );

	// overrides
	virtual int32_t MP_STDCALL initialize();

private:
 	void onSetTrigger();

	bool previousTrigger;

 	BoolGuiPin trigger;
 	IntGuiPin command;
 	StringGuiPin commandList;
 	StringGuiPin filename;
};

#endif


