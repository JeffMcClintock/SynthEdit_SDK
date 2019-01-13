#pragma once
#ifndef BOOLINVERTERGUI_H_INCLUDED
#define BOOLINVERTERGUI_H_INCLUDED

#include "MP_SDK_GUI.h"

class BoolInverterGui : public MpGuiBase
{
public:
	BoolInverterGui( IMpUnknown* host );
	virtual int32_t MP_STDCALL initialize() override;

private:
 	void onSetInput();
 	void onSetOutput();
 	BoolGuiPin input;
 	BoolGuiPin output;
};

#endif


