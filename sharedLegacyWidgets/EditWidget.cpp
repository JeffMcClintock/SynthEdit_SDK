#include "EditWidget.h"
#include <algorithm>
#include "../shared/unicode_conversion.h"
#include "../se_sdk3/MpString.h"

using namespace std;
using namespace gmpi;
using namespace gmpi_gui;
using namespace gmpi_sdk;
using namespace JmUnicodeConversions;
using namespace GmpiDrawing;
using namespace GmpiDrawing_API;

EditWidget::EditWidget() :
readOnly(false)
, onTextEntryCompeteEvent([this](int32_t result) -> void { this->OnTextEnteredComplete(result); })
, m_visable_chars(-1) // signifies to use default for datatype
{
}

void EditWidget::OnRender(Graphics& g)
{
	Rect r = position;

	// Background Fill.
	auto brush = g.CreateSolidColorBrush(Color::White);
	g.FillRectangle(r, brush);

	// Outline.
	{
		const float penWidth = 1;
		auto factory = g.GetFactory();
		auto geometry = factory.CreatePathGeometry();
		auto sink = geometry.Open();

		float offset = 0.5f;
		r.top += offset;
		r.left += offset;
		r.bottom -= offset;
		r.right -= offset;

		Point p(r.left, r.top);

		sink.BeginFigure(p, FigureBegin::Filled);

		sink.AddLine(Point(r.right, r.top));
		sink.AddLine(Point(r.right, r.bottom));
		sink.AddLine(Point(r.left, r.bottom));

		sink.EndFigure();
		sink.Close();

		brush.SetColor(Color::FromBytes(100, 100, 100));

		g.DrawGeometry(geometry, brush, penWidth);
	}

	// Current selection text.
	brush.SetColor(Color::FromBytes(0, 0, 50));

	static const int border = 2;
	Rect textRect(r);
	textRect.left += border;
	textRect.right -= border;

	// Note: SE sizes these too short to fit the text properly with ascenders and decenders.
	// Shift the rect up if text require more vertical space than avail.
//	auto text_size = dtextFormat.GetTextExtentU("M", 1);
	/*
	 *
	float shift = (std::max)(0.0f, charHeight - textRect.getHeight());
	textRect.top -= shift;
	textRect.bottom -= shift;
	 */
	auto dtextFormat = GetTextFormat(getHost(), getGuiHost(), "Custom:EditWidget");
	dtextFormat.SetParagraphAlignment(GmpiDrawing::ParagraphAlignment::Center);
	dtextFormat.SetWordWrapping(WordWrapping::NoWrap); // prevent word wrapping into two lines that don't fit box.
	auto brush2 = g.CreateSolidColorBrush(Color::FromBytes(0, 0, 255));
	g.DrawTextU(text, dtextFormat, textRect, brush2);
}

void EditWidget::Init(const char* style)
{
	FontMetadata* origStyle = nullptr;
	GetTextFormat(getHost(), getGuiHost(), style, &origStyle);

	FontMetadata f = *origStyle;
	f.setTextAlignment(GmpiDrawing::TextAlignment::Leading); // Left
//	f.SetParagraphAlignment(GmpiDrawing::ParagraphAlignment::Center);

	RegisterCustomTextFormat(getHost(), getGuiHost(), "Custom:EditWidget", &f);
}

GmpiDrawing::Size EditWidget::getSize()
{
	int vc = m_visable_chars;

	if( vc == -1 ) // use default
	{
		if( false )// m_datatype == DT_TEXT )
		{
			vc = 6;
		}
		else
		{
			vc = 4;
		}
	}

	FontMetadata* fontData = nullptr;
	GetTextFormat(getHost(), getGuiHost(), "Custom:EditWidget", &fontData);

	Size text_size((float)fontData->pixelWidth_, (float)fontData->pixelHeight_);

	float minWidth = 4;
	if( vc > 0 )
	{
		text_size.width += 2;
		text_size.width *= vc;
// not in  CMyEdit::SeMeasure():		text_size.height += 2; // allow for margins and window border

		minWidth = ( std::max )((float)fontData->pixelWidth_, minWidth );
	}

	return text_size;
}

bool EditWidget::onPointerDown(int32_t flags, GmpiDrawing_API::MP1_POINT point)
{
	return true;
}

void EditWidget::onPointerUp(int32_t flags, GmpiDrawing_API::MP1_POINT point)
{
	if( readOnly )
		return;

	GmpiDrawing::Rect r = getRect();
	gmpi_sdk::mp_shared_ptr<gmpi_gui::IMpPlatformText> returnObject;
	getGuiHost()->createPlatformTextEdit(&r, returnObject.getAddressOf());

	if( returnObject == 0 )
	{
		return;
	}

	returnObject->queryInterface(SE_IID_GRAPHICS_PLATFORM_TEXT, nativeEdit.asIMpUnknownPtr());

	if( nativeEdit == 0 )
	{
		return;
	}

	// TODO: !!! set font (at least height).
	nativeEdit->SetAlignment((int32_t) GmpiDrawing::TextAlignment::Trailing);
	nativeEdit->SetText(text.c_str());

	nativeEdit->ShowAsync(&onTextEntryCompeteEvent);
}

void EditWidget::OnTextEnteredComplete(int32_t result)
{
	if (result == gmpi::MP_OK)
	{
		MpString s;
		nativeEdit->GetText(s.getUnknown());

		SetText(s.str());

		OnChangedEvent(s.str().c_str());
	}

	nativeEdit = nullptr; // release it.
}