#include "./ListEntry4Gui.h"
#include <string>
#include <algorithm>
#include "../se_sdk3/it_enum_list.h"
#include "../shared/unicode_conversion.h"

using namespace gmpi;
using namespace gmpi_gui;
using namespace JmUnicodeConversions;
using namespace GmpiDrawing;
using namespace GmpiDrawing_API;

GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, ListEntry4Gui, L"SE List Entry4" );

ListEntry4Gui::ListEntry4Gui()
{
	// initialise pins.
	initializePin(0, pinChoice, static_cast<MpGuiBaseMemberPtr2>(&ListEntry4Gui::redraw) );
	initializePin(1, pinItemList, static_cast<MpGuiBaseMemberPtr2>(&ListEntry4Gui::redraw));
	initializePin(2, pinStyle, static_cast<MpGuiBaseMemberPtr2>(&ListEntry4Gui::onSetStyle) );
	initializePin(3, pinWriteable );
	initializePin(4, pinGreyed, static_cast<MpGuiBaseMemberPtr2>(&ListEntry4Gui::redraw) );
	initializePin(5, pinHint );
	initializePin(6, pinMenuItems );
	initializePin(7, pinMenuSelection );
}

int32_t ListEntry4Gui::onPointerDown(int32_t flags, GmpiDrawing_API::MP1_POINT point)
{
	// Let host handle right-clicks.
	if ((flags & GG_POINTER_FLAG_FIRSTBUTTON) == 0)
	{
		return gmpi::MP_OK; // Indicate successful hit, so right-click menu can show.
	}

	if (pinWriteable == false)
		return gmpi::MP_UNHANDLED;

	GmpiGui::GraphicsHost host(getGuiHost());
	nativeMenu = host.createPlatformMenu(Point(0, 0));
	// wrong nativeMenu.SetAlignment(GmpiDrawing::TextAlignment::Trailing);

	it_enum_list itr(pinItemList);

	for (itr.First(); !itr.IsDone(); itr.Next())
	{
		nativeMenu.AddItem(WStringToUtf8(itr.CurrentItem()->text).c_str(), itr.CurrentItem()->value);
	}

	nativeMenu.ShowAsync([this](int32_t result) -> void { this->OnPopupmenuComplete(result); });

	return gmpi::MP_HANDLED;
}

std::string ListEntry4Gui::getDisplayText()
{
	std::string txt;
	it_enum_list itr(pinItemList);
	if (itr.FindValue(pinChoice))
	{
		txt = WStringToUtf8(itr.CurrentItem()->text);
	}
	return txt;
}

void ListEntry4Gui::OnPopupmenuComplete(int32_t result)
{
	if (result == gmpi::MP_OK)
	{
		pinChoice = nativeMenu.GetSelectedId();
		invalidateRect();
	}

	nativeMenu.setNull(); // release it.
}

