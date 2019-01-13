#include "TextEditWidget.h"
#include <algorithm>
#include "../se_sdk3/MpString.h"

using namespace gmpi;
using namespace gmpi_gui;
using namespace gmpi_sdk;
using namespace GmpiDrawing;

void TextEditWidget::OnRender(Graphics& dc)
{
	Rect r = position;

	// Background Fill.
	auto brush = dc.CreateSolidColorBrush(Color::FromBytes(238, 238, 238));
	dc.FillRectangle(r, brush);

	// Outline.
	Rect borderRect(r);
	borderRect.Deflate(0.5f);
	brush.SetColor(Color::FromBytes(150, 150, 150));
	dc.DrawRectangle(borderRect, brush);

//	brush.SetColor(typeface_->getColor());
	brush.SetColor(Color::Black);

	static const float border = 1;
	Rect textRect(r);
	textRect.top -= 1; // emulate GDI drawing
	textRect.left += border + 1;
	textRect.right -= border;

	auto directXOffset = typeface_->getLegacyVerticalOffset();
	textRect.top += directXOffset;
	textRect.bottom += directXOffset;

	// Because of caching, and modules overriding algnment, this textformat could have *any* alignment. Always set it explicity.
	dtextFormat.SetTextAlignment(alignment);

	dc.DrawTextU(text, dtextFormat, textRect, brush, (int32_t)DrawTextOptions::Clip);
}

void TextEditWidget::Init(const char* style)
{
	style_ = style;
	if (dtextFormat.isNull())
	{
//		auto font = GetFont(getHost(), getGuiHost(), style, &typeface_);
		dtextFormat = GetTextFormat(patchMemoryHost_, guiHost_, style , &typeface_); // getGuiHost()->CreateTextFormat(font->);

		alignment = TextAlignment::Leading; // Left
		dtextFormat.SetParagraphAlignment(ParagraphAlignment::Center);
		dtextFormat.SetWordWrapping(WordWrapping::NoWrap); // prevent word wrapping into two lines that don't fit box.
	}
}

GmpiDrawing::Size TextEditWidget::getSize()
{
//	return Size(8.0f, ceilf(typeface_->pixelHeight_) + 4); // + GetSystemMetrics(SM_CYEDGE) * 4 + hack.
	return Size(8.0f, ceilf(typeface_->pixelHeight_));
														   /*
	auto text_size= dtextFormat.GetTextExtentU(text);

	text_size.height = FontCache::instance()->getOriginalPixelHeight(style_);

	// allow for margins.
	text_size.height += 2;
	text_size.width += 2;

	return text_size;
*/
}

bool TextEditWidget::onPointerDown(int32_t flags, GmpiDrawing_API::MP1_POINT point)
{
	// interested only in right-mouse clicks.
	return ((flags & (gmpi_gui_api::GG_POINTER_FLAG_FIRSTBUTTON | gmpi_gui_api::GG_POINTER_FLAG_NEW)) == (gmpi_gui_api::GG_POINTER_FLAG_FIRSTBUTTON | gmpi_gui_api::GG_POINTER_FLAG_NEW));
}

void TextEditWidget::OnPopupmenuComplete(int32_t result)
{
	if (result == gmpi::MP_OK)
	{
		OnChangedEvent(nativeFileDialog.GetText());
	}

	nativeFileDialog.setNull(); // release it.
}

void TextEditWidget::onPointerUp(int32_t flags, GmpiDrawing_API::MP1_POINT point)
{
	GmpiGui::GraphicsHost host(getGuiHost());
	nativeFileDialog = host.createPlatformTextEdit(position);
	nativeFileDialog.SetText(text);
	nativeFileDialog.SetAlignment(GmpiDrawing::TextAlignment::Trailing);

	nativeFileDialog.ShowAsync(&onPopupMenuCompleteEvent);
}