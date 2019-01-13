#ifndef INCREMENT3GUI_H_INCLUDED
#define INCREMENT3GUI_H_INCLUDED

#include "MP_SDK_GUI.h"

class Increment3Gui : public MpGuiBase
{
public:
	Increment3Gui(IMpUnknown* host);

private:
 	void onSetIncrement();
 	void onSetDecrement();
	void nextValue( int direction );

 	IntGuiPin choice;
	StringGuiPin itemList;
 	BoolGuiPin increment;
 	BoolGuiPin decrement;
 	BoolGuiPin wrap;
};

#endif


