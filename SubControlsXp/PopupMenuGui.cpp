#include "./PopupMenuGui.h"
#include "../se_sdk3/Drawing.h"
#include "../shared/unicode_conversion.h"

using namespace gmpi;
using namespace gmpi_gui;
using namespace GmpiDrawing;
using namespace JmUnicodeConversions;

GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, PopupMenuGui, L"SE Popup Menu XP" );

PopupMenuGui::PopupMenuGui()
{
	// initialise pins.
	initializePin( pinChoice, static_cast<MpGuiBaseMemberPtr2>(&PopupMenuGui::redraw) );
	initializePin( pinItemList, static_cast<MpGuiBaseMemberPtr2>(&PopupMenuGui::redraw) );
	initializePin( pinStyle, static_cast<MpGuiBaseMemberPtr2>(&PopupMenuGui::onSetStyle) );
	initializePin( pinWriteable );
	initializePin( pinGreyed, static_cast<MpGuiBaseMemberPtr2>(&PopupMenuGui::redraw) );
	initializePin( pinHint );
	initializePin( pinMenuItems );
	initializePin( pinMenuSelection);
	initializePin( pinHeading, static_cast<MpGuiBaseMemberPtr2>(&PopupMenuGui::redraw) );
	initializePin(pinMomentary);
	initializePin(pinEnableSpecialStrings);
}

int32_t MP_STDCALL PopupMenuGui::onPointerDown(int32_t flags, GmpiDrawing_API::MP1_POINT point)
{
	// Let host handle right-clicks.
	if ((flags & GG_POINTER_FLAG_FIRSTBUTTON) == 0)
	{
		return gmpi::MP_OK; // Indicate successful hit, so right-click menu can show.
	}

	if (pinWriteable == false)
		return gmpi::MP_OK; // Indicate successful hit.

	setCapture(); // prevent mouse-up going to menu (and dismissing it).

	return gmpi::MP_OK;
}

int32_t MP_STDCALL PopupMenuGui::onPointerUp(int32_t flags, GmpiDrawing_API::MP1_POINT point)
{
	if (getCapture())
	{
		releaseCapture();

		GmpiGui::GraphicsHost host(getGuiHost());
		nativeMenu = host.createPlatformMenu(Point(0, 0));
		nativeMenu.SetAlignment(TextAlignment::Leading);

		it_enum_list itr(pinItemList);

		const int popupMenuWrapRowCount = 32;
		int vertical_size = 0; // for collumns on tall menus.
		for (itr.First(); !itr.IsDone(); itr.Next())
		{
			int32_t flags = itr.CurrentItem()->value == pinChoice ? gmpi_gui::MP_PLATFORM_MENU_TICKED : 0;
			if (vertical_size++ == popupMenuWrapRowCount)
			{
				flags |= gmpi_gui::MP_PLATFORM_MENU_BREAK;
				vertical_size = 1;
			}

			auto& txt = itr.CurrentItem()->text;

			if (pinEnableSpecialStrings.getValue() && txt.size() > 3)
			{
				int i;
				for (i = 1; i < 4; ++i)
				{
					if (txt[0] != txt[i])
						break;
				}

				if (i == 4) // First 4 chars the same.
				{
					switch (txt[0])
					{
					case L'-':
						flags |= gmpi_gui::MP_PLATFORM_MENU_SEPARATOR;
						break;

					case L'|':
						flags |= gmpi_gui::MP_PLATFORM_MENU_BREAK;
						vertical_size = 1;
						break;

					case L'>':
						flags = gmpi_gui::MP_PLATFORM_SUB_MENU_BEGIN; // ignore ticked flag.
						txt = txt.substr(4);
						vertical_size = 0; // not quite right, loses count on parent menu.
						break;

					case L'<':
						flags |= gmpi_gui::MP_PLATFORM_SUB_MENU_END;
						break;
					}
				}
			}

			nativeMenu.AddItem(txt, itr.CurrentItem()->value, flags);
		}

		nativeMenu.ShowAsync([this](int32_t result) -> void { this->OnPopupComplete(result); });
	}
	return gmpi::MP_OK;
}

std::string PopupMenuGui::getDisplayText()
{
	return WStringToUtf8(pinHeading.getValue());
}

void PopupMenuGui::OnPopupComplete(int32_t result)
{
	if (result == gmpi::MP_OK)
	{
		pinChoice = nativeMenu.GetSelectedId();

		if (pinMomentary)
		{
			pinChoice = -1;
		}
	}

	nativeMenu.setNull(); // release it.
}




