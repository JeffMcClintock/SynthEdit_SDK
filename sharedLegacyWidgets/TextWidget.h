#pragma once

/*
#include "TextWidget.h"
*/

#include "Widget.h"
#include "../se_sdk3/mp_sdk_gui2.h"
#include "../shared/ImageMetadata.h"
#include "../shared/FontCache.h"

class TextWidget :
	public Widget, public FontCacheClient
{
	std::string text;
	GmpiDrawing::TextFormat dtextFormat;
	FontMetadata* typeface_;
	std::string style_;
	GmpiDrawing::TextAlignment alignment;
	GmpiDrawing_API::MP1_COLOR backgroundColor;

public:
	TextWidget(gmpi::IMpUnknown* host = nullptr) :
		typeface_(0)
	{
		if (host)
			setHost(host);
	}

	TextWidget(gmpi::IMpUnknown* host, const char* style, const char* text = "") :
		typeface_(0)
	{
		setHost(host);
		SetText(text);
		Init(style);
	}

	virtual void OnRender(GmpiDrawing::Graphics& dc) override;
	void Init(const char* style);
	void SetText(std::string utf8String)
	{
		dirty |= utf8String != text;
		text = utf8String;
	}
	const std::string& GetText()
	{
		return text;
	}
	virtual GmpiDrawing::Size getSize() override;

	void setCentered()
	{
		alignment = GmpiDrawing::TextAlignment::Center;
	}
	void setLeftAligned()
	{
		alignment = GmpiDrawing::TextAlignment::Leading;
	}
	void setRightAligned()
	{
		alignment = GmpiDrawing::TextAlignment::Trailing;
	}

	void setBackGroundColor(GmpiDrawing::Color c)
	{
		backgroundColor = c;
	}
	
};

