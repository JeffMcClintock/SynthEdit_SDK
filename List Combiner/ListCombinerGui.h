#ifndef LISTCOMBINERGUI_H_INCLUDED
#define LISTCOMBINERGUI_H_INCLUDED

#include "MP_SDK_GUI.h"

class ListCombinerGui : public MpGuiBase
{
public:
	ListCombinerGui( IMpUnknown* host );

private:
 	void onSetItemListIn();
 	void onSetChoiceOut();

	IntGuiPin choiceA;
 	StringGuiPin itemListA;
 	IntGuiPin choiceB;
 	StringGuiPin itemListB;
 	IntGuiPin choiceOut;
 	StringGuiPin itemListOut;
 	BoolGuiPin AMomentary;
 	BoolGuiPin BMomentary;
};

#endif


