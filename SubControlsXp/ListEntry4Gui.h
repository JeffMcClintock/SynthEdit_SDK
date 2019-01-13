#ifndef LISTENTRY4GUI_H_INCLUDED
#define LISTENTRY4GUI_H_INCLUDED

#include "TextSubcontrol.h"

class ListEntry4Gui : public TextSubcontrol
{
	GmpiGui::PopupMenu nativeMenu;

public:
	ListEntry4Gui();

	// overrides.
	virtual int32_t MP_STDCALL onPointerDown(int32_t flags, GmpiDrawing_API::MP1_POINT point);
	virtual std::string getDisplayText() override;

private:
	void OnPopupmenuComplete(int32_t result);
 	IntGuiPin pinChoice;
 	StringGuiPin pinItemList;
};

#endif


