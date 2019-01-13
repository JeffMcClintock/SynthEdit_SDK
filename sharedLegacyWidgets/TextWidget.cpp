#include "TextWidget.h"
#include <algorithm>
#include "../se_sdk3/MpString.h"

using namespace gmpi;
using namespace gmpi_gui;
using namespace gmpi_sdk;
using namespace GmpiDrawing;

void TextWidget::OnRender(Graphics& g)
{
	Rect r = position;

	// Background Fill.
	auto brush = g.CreateSolidColorBrush(backgroundColor);

	g.FillRectangle(r, brush);

	brush.SetColor(typeface_->getColor());

	static const float border = 1;
	Rect textRect(r);
	textRect.top -= 2; // emulate GDI drawing
	textRect.left += border;
	textRect.right -= border;

	auto directXOffset = typeface_->getLegacyVerticalOffset();
	textRect.top += directXOffset;
	textRect.bottom += directXOffset;

	// Because of caching, and modules overriding algnment, this textformat could have *any* alignment. Always set it explicity.
	dtextFormat.SetTextAlignment(alignment);

//	g.DrawTextU(text, dtextFormat, textRect, g.CreateSolidColorBrush(Color::White), (int32_t)DrawTextOptions::Clip);
	g.DrawTextU(text, dtextFormat, textRect, brush, (int32_t)DrawTextOptions::Clip);

//	_RPT4(_CRT_WARN, "DrawSize (%f,%f)\n", textRect.getWidth(), textRect.getHeight());
}

void TextWidget::Init(const char* style)
{
	style_ = style;
	if (dtextFormat.isNull())
	{
		dtextFormat = GetTextFormat(patchMemoryHost_, guiHost_, style , &typeface_);

		alignment = TextAlignment::Leading; // Left
		dtextFormat.SetParagraphAlignment(ParagraphAlignment::Center);
		dtextFormat.SetWordWrapping(WordWrapping::NoWrap); // prevent word wrapping into two lines that don't fit box.

		backgroundColor = typeface_->getBackgroundColor();
	}
}

GmpiDrawing::Size TextWidget::getSize()
{
	auto text_size= dtextFormat.GetTextExtentU(text);

//	_RPT4(_CRT_WARN, "text extent'%s'", text.c_str());
//	_RPT2(_CRT_WARN, " text_size (%f,%f) \n", text_size.width, text_size.height);

//	text_size.height = FontCache::instance()->getOriginalPixelHeight(style_);
	FontMetadata* returnMetadata;
	FontCache::instance()->GetTextFormat(getHost(), getGuiHost(), style_, &returnMetadata);
	text_size.height = (float) returnMetadata->pixelHeight_;

	// allow for margins.
//? 	text_size.height += 2;
	text_size.width += 2; // from CMyEdit3::GetMinSize()

	return text_size;
}

