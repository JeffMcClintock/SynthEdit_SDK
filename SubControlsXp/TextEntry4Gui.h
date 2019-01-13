#ifndef TEXTENTRY4GUI_H_INCLUDED
#define TEXTENTRY4GUI_H_INCLUDED

#include <functional>
#include "TextSubcontrol.h"

class TextEntry4Gui : public TextSubcontrol
{
	GmpiGui::TextEdit nativeEdit;

public:
	TextEntry4Gui();

	// overrides.
	virtual int32_t MP_STDCALL onPointerDown(int32_t flags, GmpiDrawing_API::MP1_POINT point);
	virtual int32_t MP_STDCALL onPointerUp(int32_t flags, GmpiDrawing_API::MP1_POINT point);

	virtual std::string getDisplayText() override;

private:
	void OnTextEnteredComplete(int32_t result);

	StringGuiPin pinText;
 	BoolGuiPin pinMultiline;
 	BoolGuiPin pinMouseDown;
};

#endif


