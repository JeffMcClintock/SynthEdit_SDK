#include "./TextEntry4Gui.h"
#include "../shared/unicode_conversion.h"
#include "../se_sdk3/it_enum_list.h"
#include "../se_sdk3/MpString.h"
#include <algorithm>

using namespace gmpi;
using namespace gmpi_gui;
using namespace gmpi_sdk;
using namespace JmUnicodeConversions;
using namespace GmpiDrawing;
using namespace GmpiDrawing_API;
using namespace gmpi_gui_api;

GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, TextEntry4Gui, L"SE Text Entry4" );

TextEntry4Gui::TextEntry4Gui()
{
	// initialise pins.
	initializePin( pinText, static_cast<MpGuiBaseMemberPtr2>(&TextEntry4Gui::redraw) );
	initializePin( pinStyle, static_cast<MpGuiBaseMemberPtr2>(&TextEntry4Gui::onSetStyle) );
	initializePin( pinMultiline, static_cast<MpGuiBaseMemberPtr2>(&TextEntry4Gui::redraw) );
	initializePin( pinWriteable );
	initializePin( pinGreyed, static_cast<MpGuiBaseMemberPtr2>(&TextEntry4Gui::redraw) );
	initializePin( pinHint );
	initializePin( pinMenuItems );
	initializePin( pinMenuSelection );
	initializePin( pinMouseDown );
}

int32_t TextEntry4Gui::onPointerDown(int32_t flags, GmpiDrawing_API::MP1_POINT point)
{
	// Let host handle right-clicks.
	if( ( flags & GG_POINTER_FLAG_FIRSTBUTTON ) == 0 )
	{
		return gmpi::MP_OK; // Indicate successful hit, so right-click menu can show.
	}

	setCapture();

	pinMouseDown = true;

	return gmpi::MP_OK;
}

int32_t TextEntry4Gui::onPointerUp(int32_t flags, GmpiDrawing_API::MP1_POINT point)
{
	if( !getCapture() )
	{
		return gmpi::MP_UNHANDLED;
	}

	releaseCapture();

	pinMouseDown = false;

	if (pinWriteable == false)
		return gmpi::MP_OK;

	GmpiGui::GraphicsHost host(getGuiHost());
	nativeEdit = host.createPlatformTextEdit(getRect());
	nativeEdit.SetAlignment(TextAlignment::Trailing);
	nativeEdit.SetTextSize((float) GetFontMetatdata(pinStyle)->pixelHeight_);
	nativeEdit.SetText(WStringToUtf8(pinText).c_str());
	nativeEdit.ShowAsync([this](int32_t result) -> void { this->OnTextEnteredComplete(result); });

	return gmpi::MP_OK;
}

std::string TextEntry4Gui::getDisplayText()
{
	return WStringToUtf8(pinText.getValue());
}

void TextEntry4Gui::OnTextEnteredComplete(int32_t result)
{
	if (result == gmpi::MP_OK)
	{
		pinText = Utf8ToWstring( nativeEdit.GetText() );
		invalidateRect();
	}

	nativeEdit.setNull(); // release it.
}
/*
int32_t TextEntry4Gui::OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext)
{
	GmpiDrawing::Graphics dc2(drawingContext);

	Rect r = getRect();

	auto textdata = GetFontMetatdata(pinStyle);

	// Background Fill. Currently fills behind frame line too (could be more efficient).
	auto brush = dc2.CreateSolidColorBrush(textdata->getBackgroundColor());
	dc2.FillRectangle(r, brush);

#if 0
	// testing. draw bounds.
	{
		auto brush2 = dc2.CreateSolidColorBrush(Color(0xff0000u, 0.5f));
		dc2.FillRectangle(r, brush2);
	}
#endif


	/* TextEntry3 didn't draw any outline
	// Outline.
//	if( rand() & 1)
	{
		const float penWidth = 1;

		gmpi_sdk::mp_shared_ptr<GmpiDrawing_API::IMpFactory> factory;
		dc->GetFactory(&factory.get());
		gmpi_sdk::mp_shared_ptr<IMpPathGeometry> geometry;
		factory->CreatePathGeometry(&geometry.get());
		gmpi_sdk::mp_shared_ptr<IMpGeometrySink> sink;
		geometry->Open(&sink.get());

		GmpiDrawing::Point p(r.top, r.left);

		sink->BeginFigure(p, MP1_FIGURE_BEGIN_HOLLOW);

		sink->AddLine(Point(r.left, r.top));
		sink->AddLine(Point(r.right, r.top));
		sink->AddLine(Point(r.right, r.bottom));
		sink->AddLine(Point(r.left, r.bottom));

		sink->EndFigure(MP1_FIGURE_END_CLOSED);
		sink->Close();

		brush.SetColor(Color::FromBytes(150, 150, 150));

		dc->DrawGeometry(geometry, brush.Get(), penWidth, nullptr);
	}
	* /

	// Current selection text.
	brush.SetColor(textdata->getColor());

	Rect textRect(r);
	textRect.Deflate((float) border);

	auto textformat = GetTextFormat(pinStyle);
	textformat.SetTextAlignment(TextAlignment::Center);

	dc2.DrawTextU(pinText.getValue(), textformat, textRect, brush, (int32_t) DrawTextOptions::Clip);

	return gmpi::MP_OK;
}
*/
