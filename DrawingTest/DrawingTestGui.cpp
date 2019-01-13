#include "./DrawingTestGui.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include "../shared/fast_gamma.h"

using namespace std;
using namespace gmpi;
using namespace gmpi_gui;
using namespace GmpiDrawing;

GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, DrawingTestGui, L"SE Drawing Test" );

DrawingTestGui::DrawingTestGui()
{
	initializePin(pinTestType, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestGui::refresh));
	initializePin(pinFontface, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestGui::refresh));
	initializePin(pinFontsize, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestGui::refresh));
	initializePin(pinText, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestGui::refresh));
	initializePin(pinApplyAlphaCorrection, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestGui::refresh));
	initializePin(pinAdjust, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestGui::refresh));	
}

void DrawingTestGui::refresh()
{
	invalidateRect();
}

void DrawingTestGui::MyApplyGammaCorrection(Bitmap& bitmap)
{
	auto pixelsSource = bitmap.lockPixels(true);
	auto imageSize = bitmap.GetSize();
	int totalPixels = (int)imageSize.height * pixelsSource.getBytesPerRow() / sizeof(uint32_t);

	uint8_t* sourcePixels = pixelsSource.getAddress();
	const float gamma = 2.2f;
	for (int i = 0; i < totalPixels; ++i)
	{
		int alpha = sourcePixels[3];

		if (alpha != 0 && alpha != 255)
		{
			float bitmapAlpha = alpha / 255.0f;

			// Calc pixel lumination (linear).
			float components[3];
			float foreground = 0.0f;
			for (int c = 0; c < 3; ++c)
			{
				float pixel = sourcePixels[c] / 255.0f;
				pixel /= bitmapAlpha; // un-premultiply
				pixel = powf(pixel, gamma);
				components[c] = pixel;
				//						foreground += pixel;
				//						foreground = (std::max)(foreground, pixel);
			}
			//					foreground /= 3.0f; // average pixels.
			//					foreground = 0.2126 * components[2] + 0.7152 * components[1] + 0.0722 * components[0]; // Luminance.
			foreground = 0.3333f * components[2] + 0.3333f * components[1] + 0.3333f * components[0]; // Average. Much the same as Luminance, better on Blue.
/*
			if (pinApplyAlphaCorrection)
			{
//				foreground = (std::min)((std::min)(components[2], components[1]), components[0]);
				foreground = (std::max)((std::max)(components[2], components[1]), components[0]);
			}
*/

			float blackAlpha = 1.0f - powf(1.0f - bitmapAlpha, 1.0 / gamma);
			float whiteAlpha = powf(bitmapAlpha, 1.0f / gamma);

			float mix = powf(foreground, 1.0f / gamma);

			float bitmapAlphaCorrected = blackAlpha * (1.0f - mix) + whiteAlpha * mix;

			for (int c = 0; c < 3; ++c)
			{
				float pixel = components[c];

				// Alpha is calculated on average forground intensity, need to tweak components that are brighter than average to prevent themgetting too dim.
				//float IntensityError = pixel / foreground;
				//pixel *= IntensityError;

				pixel = powf(pixel, 1.0f / gamma); // linear -> sRGB space.
				pixel *= bitmapAlphaCorrected; // premultiply
				sourcePixels[c] = (std::min)(255, (int)(pixel * 255.0f + 0.5f));
			}

			int alphaVal = (int)(bitmapAlphaCorrected * 255.0f + 0.5f);
			sourcePixels[3] = alphaVal;
		}
		sourcePixels += sizeof(uint32_t);
	}
}

void DrawingTestGui::drawGammaTest(GmpiDrawing::Graphics& g)
{
	const int resolution = 1; // 1 or 10
	const float gamma = 2.2f;
	float foregroundColor[3] = { 1, 1, 1 }; // BGR

	// create bitmap with every intensity vs every alpha.
	auto bitmapMem = GetGraphicsFactory().CreateImage(100 + resolution, 100 + resolution);
	{
		auto pixelsSource = bitmapMem.lockPixels(true);
		auto imageSize = bitmapMem.GetSize();
		int totalPixels = (int)imageSize.height * pixelsSource.getBytesPerRow() / sizeof(uint32_t);

		uint8_t* sourcePixels = pixelsSource.getAddress();

		float x = 0;
		float foreground = 0.0f;

		for (float x = 0 ; x <= 100 ; x += resolution)
		{
			float alpha = 1.0f;
			for (float y = 0; y <= 100; y += resolution)
			{
				int alphaVal = (int)(alpha * 255.0f + 0.5f);

				int pixelVal[3];
				for(int i = 0 ; i < 3; ++i)
				{
					// apply alpha in lin space.
					float fg = foregroundColor[i] * foreground;
					fg *= alpha; // pre-multiply.

					// then convert to SRGB.
					pixelVal[i] = se_sdk::FastGamma::float_to_sRGB(fg);
				}

				// Fill in square with calulated color.
				for (int xi = x; xi < x + resolution; ++xi)
				{
					for (int yi = y; yi < y + resolution; ++yi)
					{
						uint8_t* pixel = sourcePixels + ((int) sizeof(uint32_t) * (xi + yi * (int)(imageSize.width)));

						for (int i = 0; i < 3; ++i)
							pixel[i] = pixelVal[i];

						pixel[3] = alphaVal;
					}
				}

				alpha -= resolution / 100.0f;
			}
			foreground += resolution / 100.0f;
		}
	}

	if (pinApplyAlphaCorrection)
	{
		bitmapMem.ApplyAlphaCorrection();
		//MyApplyGammaCorrection(bitmapMem);
	}

	int x1 = 0;
	int y1 = 0;
	auto brush = g.CreateSolidColorBrush(Color::Transparent());
	// 8 backgrounds. down left, then down right. Black to White.
	for (double background = 0.0; background < 1.05; background += 1.0 / 7.0)
	{
		// Ideal Gamma test rectangle.
		float foreground = 0.0f;
		for (float x = 0; x <= 100; x += resolution)
		{
			float alpha = 1.0f;
			for (float y = 0; y <= 100; y += resolution)
			{
				float gammaCorrectForeground[3];
				for (int i = 0; i < 3; ++i)
				{
					float blend = foregroundColor[i] * foreground * alpha + (1.0f - alpha) * background;
					gammaCorrectForeground[i] = blend; // se_sdk::FastGamma::linearToSrgb(blend);
				}

				brush.SetColor(Color(gammaCorrectForeground[2], gammaCorrectForeground[1], gammaCorrectForeground[0], 1.0f));
				g.FillRectangle(x1 + x, y1 + y, x1 + x + resolution, y1 + y + resolution, brush);

				alpha -= resolution / 100.0f;
			}
			foreground += resolution / 100.0f;
		}

		// Actual blend
		float gammaCorrectBackground = background; // se_sdk::FastGamma::linearToSrgb(background);  // powf(background, 1.0f / gamma);
		brush.SetColor(Color(gammaCorrectBackground, gammaCorrectBackground, gammaCorrectBackground, 1.0f));
		g.FillRectangle(Rect(x1 + 115, y1, x1 + 215 + resolution, y1 + 100 + resolution), brush);
		g.DrawBitmap(bitmapMem, Point(x1 + 115, y1), Rect(0, 0, 100 + resolution, 100 + resolution));
		
		y1 += 115;
		if (y1 > 430)
		{
			y1 = 0;
			x1 += 250;
		}
	}
}

void DrawingTestGui::drawGradient(GmpiDrawing::Graphics& g)
{
	auto textFormat = g.GetFactory().CreateTextFormat();
	auto textBrush = g.CreateSolidColorBrush(Color::Orange);

	const int resolution = 1; // 1 or 10
	const float gamma = 2.2f;
	float foregroundColor[3] = { 1, 1, 1 }; // BGR

	const int count = 256;
	const int height = 40;
	// create bitmap with every intensity.
	auto bitmapMem = GetGraphicsFactory().CreateImage(count, count);
	{
		auto pixelsSource = bitmapMem.lockPixels(true);
		auto imageSize = bitmapMem.GetSize();
		int totalPixels = (int)imageSize.height * pixelsSource.getBytesPerRow() / sizeof(uint32_t);

		uint8_t* sourcePixels = pixelsSource.getAddress();

		float x = 0;
		float foreground = 0.0f;
		float alpha = 1.0f;

		for (int x = 0; x < count; ++x)
		{
			for (int y = 0; y < height; ++y)
			{
				int alphaVal = (int)(alpha * 255.0f + 0.5f);

				int pixelVal[3];
				for (int i = 0; i < 3; ++i)
				{
					// apply alpha in lin space.
					//float fg = foregroundColor[i] * foreground;
					//fg *= alpha; // pre-multiply.

								 // then convert to SRGB.
					pixelVal[i] = x; // se_sdk::FastGamma::float_to_sRGB(fg);
				}

				uint8_t* pixel = sourcePixels + ((int) sizeof(uint32_t) * (x + y * (int)(imageSize.width)));

				for (int i = 0; i < 3; ++i)
					pixel[i] = pixelVal[i];

				pixel[3] = alphaVal;
			}
		}
	}

	if (pinApplyAlphaCorrection)
	{
//		bitmapMem.ApplyAlphaCorrection();
		//MyApplyGammaCorrection(bitmapMem);
	}

	g.DrawBitmap(bitmapMem, Point(0,0), Rect(0, 0, count, height));
	g.DrawTextU("Bitmap RGB", textFormat, 0.0f, 0.0f, textBrush);

	// Use brushes to draw every sRGB intensity.
	int x1 = 0;
	int y1 = 50;
	auto brush = g.CreateSolidColorBrush(Color::Transparent());

	for (float x = 0; x < count; ++x)
	{
		for (float y = 0; y < height; ++y)
		{
			brush.SetColor(Color::FromBytes(x,x,x));
			g.FillRectangle(x1 + x, y1 + y, x1 + x + 1, y1 + y + 1, brush);
		}
	}
	g.DrawTextU("Brush RGB", textFormat, x1, y1, textBrush);

	y1 = 100;

	for (float x = 0; x < count; ++x)
	{
		for (float y = 0; y < height; ++y)
		{
			brush.SetColor(Color(x / 256.0f, x / 256.0f, x / 256.0f));
			g.FillRectangle(x1 + x, y1 + y, x1 + x + 1, y1 + y + 1, brush);
		}
	}
	g.DrawTextU("Brush Linear", textFormat, x1, y1, textBrush);
}

int32_t DrawingTestGui::OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext)
{
	GmpiDrawing::Graphics g(drawingContext);

	auto r = getRect();


	if (pinTestType == 2)
	{
		drawGammaTest(g);
		return MP_OK;
	}
	if (pinTestType == 3)
	{
		drawGradient(g);
		return MP_OK;
	}
	
	// Background Fill.
	auto brush = g.CreateSolidColorBrush(Color::White);
	g.FillRectangle(r, brush);

	if (pinTestType == 0)
	{
		const char* typefaces[] = { "Segoe UI", "Arial", "Courier New", "Times New Roman" , "MS Sans Serif" };

		float x = 10.5;
		Rect textRect;
		textRect.left = x;

		textRect.top = 200.5;

		const char* fontFace = typefaces[pinFontface];

		TextFormat dtextFormat = g.GetFactory().CreateTextFormat((float)pinFontsize.getValue(), fontFace);

		dtextFormat.SetTextAlignment(TextAlignment::Leading); // Left
		dtextFormat.SetParagraphAlignment(ParagraphAlignment::Center);
		dtextFormat.SetWordWrapping(WordWrapping::NoWrap); // prevent word wrapping into two lines that don't fit box.

		string text = fontFace;
		if (!pinText.getValue().empty())
		{
			text = pinText; // .getValue();
		}

		auto textSize = dtextFormat.GetTextExtentU(text);
		textRect.bottom = textRect.top + textSize.height;
		textRect.right = textRect.left + textSize.width;

		brush.SetColor(Color(0.7f, 0.7f, 0.7f));
		g.FillRectangle(textRect, brush);

		GmpiDrawing_API::MP1_FONT_METRICS fontMetrics;
		dtextFormat.GetFontMetrics(&fontMetrics);
//		_RPT1(_CRT_WARN, "fontMetrics.ascent    %f\n", fontMetrics.ascent);
//		_RPT1(_CRT_WARN, "fontMetrics.descent   %f\n", fontMetrics.descent);
//		_RPT1(_CRT_WARN, "fontMetrics.capHeight %f\n", fontMetrics.capHeight);
//		_RPT1(_CRT_WARN, "fontMetrics.xHeight   %f\n", fontMetrics.xHeight);
//		_RPT1(_CRT_WARN, "fontMetrics.lineGap   %f\n", fontMetrics.lineGap);

		brush.SetColor(Color::LightBlue);
		float y = textRect.bottom - fontMetrics.descent - fontMetrics.ascent;
		g.DrawLine(Point(textRect.left - 3, y), Point(textRect.right + 3, y), brush);

		brush.SetColor(Color::Coral);
		y = textRect.bottom - fontMetrics.descent;
		g.DrawLine(Point(textRect.left - 3, y), Point(textRect.right + 3, y), brush);

		brush.SetColor(Color::AliceBlue);
		y = textRect.bottom;
		g.DrawLine(Point(textRect.left - 3, y), Point(textRect.right + 3, y), brush);


		// cap-height.
		brush.SetColor(Color::Green);
		y = textRect.bottom - fontMetrics.descent - fontMetrics.capHeight;
		g.DrawLine(Point(textRect.left - 3, y), Point(textRect.right + 3, y), brush);

		// x-height.
		brush.SetColor(Color::MediumBlue);
		y = textRect.bottom - fontMetrics.descent - fontMetrics.xHeight;
		g.DrawLine(Point(textRect.left - 3, y), Point(textRect.right + 3, y), brush);

		brush.SetColor(Color::Black);
		g.DrawTextU(text, dtextFormat, textRect, brush);

		return gmpi::MP_OK;
	}



	// Create font.
	int font_size_ = 12;
	char* fontFace = "Segoe UI";
	std::string text("Cat");
	float dipFontSize = (font_size_ * 72.f) / 96.f; // Points to DIPs conversion. https://social.msdn.microsoft.com/forums/vstudio/en-US/dfbadc0b-2415-4f92-af91-11c78df435b3/hwndhost-gdi-vs-directwrite-font-size

	TextFormat dtextFormat = g.GetFactory().CreateTextFormat(dipFontSize, fontFace);

	brush.SetColor(Color(0, 0, 0));

	// Paths.
	float y = 20.5;
	float w = 40;
	int i = 0;

	for (int i = 0; i < 9; ++i)
	{
		// Lines draw 'nice' at co-ord x.5
		float x = 0.5f + 45.25f * (float) i;
		float penWidth = 1;

		Rect textRect;
		textRect.bottom = y + w;
		textRect.top = y;
		textRect.left = x;
		textRect.right = x + w;

		auto geometry = g.GetFactory().CreatePathGeometry();
		auto sink = geometry.Open();

		Point p(x, y);

		sink.BeginFigure(p);

		sink.AddLine(Point(x+w, y));
		sink.AddLine(Point(x+w, y + w));
		sink.AddLine(Point(x, y+w));

		switch (i & 1)
		{
		case 0:
			sink.EndFigure();
			break;
		case 1:
			sink.EndFigure(FigureEnd::Open);
			break;
		}

		sink.Close();

		g.DrawGeometry(geometry, brush, penWidth);

		if (i != 0) // first one show default.
		{
			// Text.
			switch (i % 3)
			{
			case 0:
				dtextFormat.SetTextAlignment(TextAlignment::Leading);
				break;
			case 1:
				dtextFormat.SetTextAlignment(TextAlignment::Center);
				break;
			case 2:
				dtextFormat.SetTextAlignment(TextAlignment::Trailing);
				++penWidth;
				break;
			}

			switch ((i / 3) % 3)
			{
			case 0:
				dtextFormat.SetParagraphAlignment(ParagraphAlignment::Near); // Top
				break;
			case 1:
				dtextFormat.SetParagraphAlignment(ParagraphAlignment::Center); // Middle
				break;
			case 2:
				dtextFormat.SetParagraphAlignment(ParagraphAlignment::Far); // Bottom
				break;
			}
		}

		g.DrawTextU(text, dtextFormat, textRect, brush);
	}

	// Text Extents.
	{
		const char* words[] = { "cat", "White Noise", "the quick brown fox\njumped over the lazy dog" };
		float x = 10.5;
		float y = 120.5;
		Rect textRect;
		textRect.top = y;
		textRect.left = x;

		float penWidth = 1.0f;
		for (auto w : words)
		{
			auto textSize = dtextFormat.GetTextExtentU(w);
			textRect.bottom = textRect.top + ceilf(textSize.height);
			textRect.right = textRect.left + ceilf(textSize.width);

			brush.SetColor(Color(0.0f, 0.0f, 1.0f));
			Rect boxRect(textRect);
			boxRect.Inflate(penWidth * 0.5f);

			g.DrawRectangle(boxRect, brush, penWidth++);
			g.DrawTextU(w, dtextFormat, textRect, brush);

			textRect.top += 20;
		}
	}

	// Fonts.
	{
		const char* typefaces[] = { "Segoe UI", "Arial", "Courier New", "Times New Roman" };
		float fontSizes[] = { 10, 14, 18 , 34, 72 };

		float x = 10.5;
		Rect textRect;
		textRect.left = x;

		TextFormat dtextFormat;

		for (auto fontFace : typefaces)
		{
			textRect.top = 200.5;

			for (auto dipFontSize : fontSizes)
			{
				dtextFormat = g.GetFactory().CreateTextFormat(dipFontSize, fontFace);

				Size textSize = dtextFormat.GetTextExtentU(fontFace);
				textRect.bottom = textRect.top + textSize.height;
				textRect.right = textRect.left + textSize.width;

				brush.SetColor(Color::LightGray);
				g.FillRectangle(textRect, brush);

				GmpiDrawing_API::MP1_FONT_METRICS fontMetrics;
				dtextFormat.GetFontMetrics(&fontMetrics);

				brush.SetColor(Color::LightBlue);
				float y = textRect.bottom - fontMetrics.descent - fontMetrics.ascent;
				g.DrawLine(Point(textRect.left - 3, y), Point(textRect.right + 3, y), brush);

				brush.SetColor(Color::Coral);
				y = textRect.bottom - fontMetrics.descent;
				g.DrawLine(Point(textRect.left - 3, y), Point(textRect.right + 3, y), brush);

				brush.SetColor(Color::OrangeRed);
				y = textRect.bottom;
				g.DrawLine(Point(textRect.left - 3, y), Point(textRect.right + 3, y), brush);


				// cap-height.
				brush.SetColor(Color::Green);
				y = textRect.bottom - fontMetrics.descent - fontMetrics.capHeight;
				g.DrawLine(Point(textRect.left - 3, y), Point(textRect.right + 3, y), brush);

				// x-height.
				brush.SetColor(Color::MediumBlue);
				y = textRect.bottom - fontMetrics.descent - fontMetrics.xHeight;
				g.DrawLine(Point(textRect.left - 3, y), Point(textRect.right + 3, y), brush);

				brush.SetColor(Color::Black);
				g.DrawTextU(fontFace, dtextFormat, textRect, brush);

				textRect.top += dipFontSize * 2;
			}

			textRect.left += 120;
		}
#if 0
		// determin relationship between font-size and bounding box.
		for (auto fontFace : typefaces)
		{
			textRect.top = 200.5;

			_RPT1(_CRT_WARN, "%s----------------------------\n", fontFace);

			for (int fontSize = 10; fontSize < 50; ++fontSize)
			{
				dtextFormat = nullptr;
				getGuiHost()->CreateTextFormat(
					fontFace,
					NULL,
					gmpi_gui::Font::W_Regular,
					gmpi_gui::Font::S_Normal,
					gmpi_gui::Font::ST_Normal,
					fontSize,
					0,							// locale.
					&dtextFormat.get()
					);

				Size textSize;
				dtextFormat.GetTextExtentU("1Mj|", (int32_t)4, textSize);

				_RPT2(_CRT_WARN, "%d, %f\n", fontSize, textSize.height);
			}

			textRect.left += 120;
		}
#endif
	}

	// Arc test
	{
		auto geometry = g.GetFactory().CreatePathGeometry();
		auto sink = geometry.Open();

		double centerX = 200.0;
		double centerY = 200.0;
		double radiusX = 100.0;
		double radiusY = 150.0;
		double startAngle = 0.0;
		double endAngle = M_PI * 2.0;
		double stepSize = (endAngle - startAngle) * 0.1;
		bool first = true;
		for (double a = startAngle; a < endAngle; a += stepSize)
		{
			double x = centerX + sin(a) * radiusX;
			double y = centerY + cos(a) * radiusY;
			Point p(x, y);

			if (first)
			{
				first = false;
				sink.BeginFigure(p);
			}
			else
			{
				sink.AddLine(Point(x, y));

			}
		}

		if ( (endAngle - startAngle) == 0.0 ) // TODO do detect full circle
		{
			sink.EndFigure();
		}
		else
		{
			sink.EndFigure(FigureEnd::Open);
		}

		sink.Close();

		float penWidth = 1;
		g.DrawGeometry(geometry, brush, penWidth);
	}

	return gmpi::MP_OK;
}

int32_t DrawingTestGui::measure(GmpiDrawing_API::MP1_SIZE availableSize, GmpiDrawing_API::MP1_SIZE* returnDesiredSize)
{
	returnDesiredSize->width = 540;
	returnDesiredSize->height = 540;

	return gmpi::MP_OK;
}
