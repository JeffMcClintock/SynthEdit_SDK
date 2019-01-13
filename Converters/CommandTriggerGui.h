#ifndef BOOLSTOINTGUI_H_INCLUDED
#define BOOLSTOINTGUI_H_INCLUDED

#include "MP_SDK_GUI.h"
#include <vector>

class CommandTriggerGui : public MpGuiBase
{
public:
	CommandTriggerGui( IMpUnknown* host );

	virtual int32_t MP_STDCALL setPin( int32_t pinId, int32_t voice, int32_t size, void* data );
	virtual int32_t MP_STDCALL initialize();

private:
	IntGuiPin value;

	std::vector<bool> previousValues;
	int outputPinCount_;
};

#endif


