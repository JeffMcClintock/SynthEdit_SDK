#include "TextSubcontrol.h"
#include "../shared/unicode_conversion.h"

using namespace gmpi;
using namespace GmpiDrawing;
using namespace JmUnicodeConversions;

int32_t TextSubcontrol::OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext)
{
	GmpiDrawing::Graphics g(drawingContext);

	auto r = getRect();
	Rect textRect(r);
	textRect.Deflate((float)border, (float)border);

	auto textformat = GetTextFormat(pinStyle);
	auto textdata = GetFontMetatdata(pinStyle);

	// Background Fill. Currently fills behind frame line too (could be more efficient).
	auto brush = g.CreateSolidColorBrush(textdata->getBackgroundColor());
	g.FillRectangle(textRect, brush);

	if (pinGreyed == true)
		brush.SetColor(Color::Gray);
	else
		brush.SetColor(textdata->getColor());

	textformat.SetTextAlignment(textdata->getTextAlignment());

	auto directXOffset = textdata->getLegacyVerticalOffset();
	textRect.top += directXOffset;
	textRect.bottom += directXOffset;

	g.DrawTextU(getDisplayText(), textformat, textRect, brush, (int32_t)DrawTextOptions::Clip);

#if 0 // TextEntry3 didn't draw arrow.
	// Divide control into text display and drop-down button.
	float dropDownArrowRight = r.right - fontSize * 2.0f;

	textRect.right = dropDownArrowRight - border;

	// Drop-down indicator 'arrow'.
	brush->SetColor(&Color::FromBytes(110, 110, 110));
	{
		const float penWidth = 2;

		gmpi_sdk::mp_shared_ptr<GmpiDrawing_API::IMpFactory> factory;
		dc->GetFactory(&factory.get());

		gmpi_sdk::mp_shared_ptr<IMpPathGeometry> geometry;
		factory->CreatePathGeometry(&geometry.get());

		gmpi_sdk::mp_shared_ptr<IMpGeometrySink> sink;
		geometry->Open(&sink.get());

		GmpiDrawing::Point p(dropDownArrowRight, r.getHeight() * 0.5f - fontSize * 0.25f);

		sink->BeginFigure(p, MP1_FIGURE_BEGIN_HOLLOW);
		p.x += fontSize * 0.5f;
		p.y += fontSize * 0.5f;
		sink->AddLine(p);
		p.x += fontSize * 0.5f;
		p.y -= fontSize * 0.5f;
		sink->AddLine(p);

		sink->EndFigure(MP1_FIGURE_END_OPEN);
		sink->Close();
		dc->DrawGeometry(geometry, brush, penWidth);
	}
#endif

	return gmpi::MP_OK;
}

void TextSubcontrol::onSetStyle()
{
	getGuiHost()->invalidateMeasure();
}

int32_t TextSubcontrol::populateContextMenu(float x, float y, gmpi::IMpUnknown* contextMenuItemsSink)
{
	gmpi::IMpContextItemSink* sink;
	contextMenuItemsSink->queryInterface(gmpi::MP_IID_CONTEXT_ITEMS_SINK, reinterpret_cast<void**>(&sink));

	it_enum_list itr(pinMenuItems);

	for (itr.First(); !itr.IsDone(); ++itr)
	{
		sink->AddItem(WStringToUtf8(itr.CurrentItem()->text).c_str(), itr.CurrentItem()->value);
	}
	return gmpi::MP_OK;
}

int32_t TextSubcontrol::onContextMenu(int32_t selection)
{
	pinMenuSelection = selection; // send menu momentarily, then reset.
	pinMenuSelection = -1;

	return gmpi::MP_OK;
}

int32_t TextSubcontrol::getToolTip(float x, float y, gmpi::IMpUnknown * returnToolTipString)
{
	IString* returnValue = 0;

	auto hint = pinHint.getValue();

	if (hint.empty() || MP_OK != returnToolTipString->queryInterface(gmpi::MP_IID_RETURNSTRING, reinterpret_cast<void**>(&returnValue)))
	{
		return gmpi::MP_NOSUPPORT;
	}

	auto utf8ToolTip = WStringToUtf8(hint);

	returnValue->setData(utf8ToolTip.data(), (int32_t)utf8ToolTip.size());

	return gmpi::MP_OK;
}

int32_t TextSubcontrol::measure(GmpiDrawing_API::MP1_SIZE availableSize, GmpiDrawing_API::MP1_SIZE* returnDesiredSize)
{
	*returnDesiredSize = availableSize;
	const float minSize = 4;
	returnDesiredSize->height = (std::max)(returnDesiredSize->height, minSize);
	returnDesiredSize->width = (std::max)(returnDesiredSize->width, minSize);

	return gmpi::MP_OK;
}
