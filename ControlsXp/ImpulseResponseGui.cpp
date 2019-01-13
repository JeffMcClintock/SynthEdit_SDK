#include "mp_sdk_gui2.h"
#include "Drawing.h"
#include "../shared/FontCache.h"

using namespace gmpi;
using namespace GmpiDrawing;

class ImpulseResponseGui : public gmpi_gui::MpGuiGfxBase, public FontCacheClient
{
 	void onSetResults()
	{
		invalidateRect();
	}

 	BlobGuiPin pinResults;
 	FloatGuiPin pinsampleRate;

	float displayOctaves = 10.0f;
	float displayDbMax = 30.0f;
	float displayDbRange = 100.0f;
	const static int SCOPE_BUFFER_SIZE = 512;

public:
	ImpulseResponseGui()
	{
		initializePin( pinResults, static_cast<MpGuiBaseMemberPtr2>(&ImpulseResponseGui::onSetResults) );
		initializePin( pinsampleRate, static_cast<MpGuiBaseMemberPtr2>(&ImpulseResponseGui::onSetResults) );
	}

	int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext ) override
	{
		Graphics g(drawingContext);

		auto brush = g.CreateSolidColorBrush(Color::Red);

		// Processor send an odd number of samples when in "Impulse" mode (not "Frequency Response" mode).
		bool rawImpulseMode = (pinResults.rawSize() & 1) == 1;

		// PAINT. /////////////////////////////////////////////////////////////////////////////////////////////////
		auto r = getRect();
		auto width = r.right - r.left;
		auto height = r.bottom - r.top;

		g.PushAxisAlignedClip(r);

		FontMetadata* typeface_ = nullptr;
		auto dtextFormat = GetTextFormat(getHost(), getGuiHost(), "tty", &typeface_);

		dtextFormat.SetTextAlignment(TextAlignment::Leading); // Left
		dtextFormat.SetParagraphAlignment(ParagraphAlignment::Center);
		dtextFormat.SetWordWrapping(WordWrapping::NoWrap); // prevent word wrapping into two lines that don't fit box.

		float scale = height * 0.95f;
		auto mid_x = width * 0.5f;
		auto mid_y = height * 0.5f;
		float leftBorder = 20.f;

		if (typeface_->backgroundColor_ >= 0xff000000) // don't bother for transparent.
		{
			// Fill in solid background black
			auto background_brush = g.CreateSolidColorBrush(typeface_->backgroundColor_);
			g.FillRectangle(r, background_brush);
		}

		// create a dark green pen
		int darked_col = (typeface_->color_ >> 2) & 0x3f3f3f; // more darker.
		HPEN pen = CreatePen(PS_SOLID, 1, darked_col);

		// 'select' it
		int fontHeight = typeface_->pixelHeight_;
		float penWidth = 1.0f;

		brush.SetColor(typeface_->color_);
		auto brush2 = g.CreateSolidColorBrush(Color::Gray);

		float samplerate = (float)pinsampleRate.getValue();

		// Frequency labels.
//		SetTextAlign(hDC, TA_CENTER);

		if (!rawImpulseMode)
		{
			// Highest mark is rounded to nearest 10kHz.
			float topFrequency = floorf(samplerate / 20000.0f) * 10000.0f;
			float frequencyStep = 1000.0f;
			if (width < 500)
			{
				frequencyStep = 10000.0f;
			}
			float hz = topFrequency;
			float previousTextLeft = 100000.0f;
			int x = INT_MAX;
			do {
				float normalizedFrequency = 2.0f * hz / samplerate;
				float octave = (log(normalizedFrequency) - log(1.0f)) / log(2.0f);
				x = (int)(((octave + displayOctaves) * width) / displayOctaves);

				bool extendLine = false;

				// Text.
				if (samplerate > 0)
				{
					char txt[10];

					// Large values printed in kHz.
					if (hz > 999.0)
					{
						sprintf(txt, "%.0fk", hz * 0.001);
					}
					else
					{
						sprintf(txt, "%.0f", hz);
					}

					auto size = dtextFormat.GetTextExtentU(txt);

					// Ensure text don't overwrite text to it's right.
					if (x + size.width / 2 < previousTextLeft)
					{
						extendLine = true;
//						TextOut(hDC, x, height - fontHeight, txt, stringLength);
						GmpiDrawing::Rect textRect(x - size.width / 2, height - fontHeight, x + size.width / 2, height);
						g.DrawTextU(txt, dtextFormat, textRect, brush);
						previousTextLeft = x - (2 * size.width) / 3.0f; // allow for text plus whitepace.
					}
				}

				// Vertical line.
				float lineBottom = height - fontHeight;
				if (!extendLine)
				{
					lineBottom -= 2;
				}
				g.DrawLine(GmpiDrawing::Point((float) x, 0.f), GmpiDrawing::Point((float)x, lineBottom), brush2, penWidth);

				if (frequencyStep > hz * 0.99)
				{
					frequencyStep *= 0.1;
				}

				hz = hz - frequencyStep;

			} while (x > 25 && hz > 5.0);

			/* mark in octaves, possible option?
			for( float octave = 0.0 ; octave > -displayOctaves ; octave -= 1.0f )
			{
			float Hz = samplerate / ( powf( 2.0f, 1.0f - octave) );
			int x = (int) (((octave + displayOctaves) * width) / displayOctaves);

			MoveToEx(hDC, x,0, 0);
			LineTo(hDC, x, height - fontHeight - 2 );

			if( samplerate > 0 )
			{
			wchar_t txt[10];
			swprintf(txt, L"%2.0f", Hz);

			if( x < 20 )
			{
			SetTextAlign( hDC, TA_LEFT );
			}
			else
			{
			if( x > width - 20 )
			{
			SetTextAlign( hDC, TA_RIGHT );
			}
			else
			{
			SetTextAlign( hDC, TA_CENTER );
			}
			}

			TextOut(hDC, x, height - fontHeight, txt, (int) wcslen(txt));
			}
			}
			*/

			// dB labels.
			if (height > 30)
			{
//				SetTextAlign(hDC, TA_LEFT);

				float db = displayDbMax;
				float y = 0;
				while (y < height)
				{
					y = (displayDbMax - db) * scale / displayDbRange;

					g.DrawLine(GmpiDrawing::Point(leftBorder, y), GmpiDrawing::Point(width, y), brush2, penWidth);

					char txt[10];
					sprintf(txt, "%2.0f", (float)db);

//					TextOut(hDC, 0, y - fontHeight / 2, txt, (int)wcslen(txt));
					GmpiDrawing::Rect textRect(0, y - fontHeight / 2, 30, y + fontHeight / 2);
					g.DrawTextU(txt, dtextFormat, textRect, brush);

					db -= 10.0f;
				}

				// extra line at -3dB. To help check filter cuttoffs.
				db = -3.f;
				y = (displayDbMax - db) * scale / displayDbRange;

				g.DrawLine(GmpiDrawing::Point(leftBorder, y), GmpiDrawing::Point(width, y), brush2, penWidth);
			}
		}
		else
		{
			// Impulse mode.

			// Zero-line
			g.DrawLine(GmpiDrawing::Point(leftBorder, mid_y), GmpiDrawing::Point(width, mid_y), brush2, penWidth);

			// volt ticks
			for (int v = 10; v >= -10; --v)
			{
				int y = (int)(mid_y - (scale * v) * 0.1f);
				g.DrawLine(GmpiDrawing::Point(0, y), GmpiDrawing::Point(leftBorder, y), brush2, penWidth);
			}
		}

		// trace
		if (pinResults.rawSize() == sizeof(float) * SCOPE_BUFFER_SIZE)
		{
			brush2.SetColor(Color::FromBytes(0, 255, 0));
			float* capturedata = (float*)pinResults.rawData();
			int count = pinResults.rawSize() / sizeof(float);

			auto factory = g.GetFactory();

			// fill
			//auto geometry = factory.CreatePathGeometry();
			//auto sink = geometry.Open();

			// lines
			auto lineGeometry = factory.CreatePathGeometry();
			auto lineSink = lineGeometry.Open();


			{
				float dbScaler = scale / displayDbRange; // 90db Range
														 //float db = 10.f * log10f( *capturedata ) - 10.0f; // db of amplitude is * 10, db of power is * 20.
														 //float y = - db * dbScaler;

														 // Ignoring DC. Can't show on log scale.
				++capturedata;

				for (int i = 1; i < SCOPE_BUFFER_SIZE; i++)
				{
					float normalizedFrequency = (float)i / (float)SCOPE_BUFFER_SIZE;
					float octave = (log(normalizedFrequency) - log(1.0f)) / log(2.0f);
					//int x = (i * width) / SCOPE_BUFFER_SIZE;
					float x = ((octave + displayOctaves) * width) / displayOctaves;
					float db = 20.f * log10f(*capturedata) - displayDbMax;
					float y = -db * dbScaler;
					if (i == 1)
					{
						lineSink.BeginFigure(0, y, FigureBegin::Filled);
					}
					else
					{
						lineSink.AddLine(GmpiDrawing::Point(x, y));
					}
					++capturedata;
				}

				lineSink.EndFigure(FigureEnd::Open);
				lineSink.Close();
				//sink.EndFigure();
				//sink.Close();
				g.DrawGeometry(lineGeometry, brush2, penWidth);
			}
		}
		else
		{
			if (rawImpulseMode)
			{
				brush2.SetColor(Color::FromBytes(0, 255, 0)); // bright green

				auto factory = g.GetFactory();

				// fill
				//auto geometry = factory.CreatePathGeometry();
				//auto sink = geometry.Open();

				// lines
				auto lineGeometry = factory.CreatePathGeometry();
				auto lineSink = lineGeometry.Open();


				float* capturedata = (float*)pinResults.rawData();
				int count = pinResults.rawSize() / sizeof(float);
				float dbScaler = scale;

				for (int i = 0; i < count; ++i)
				{
					int x = 20 + i;
					float y = mid_y - *capturedata * scale;
					if (i == 0)
					{
						lineSink.BeginFigure(0, y, FigureBegin::Filled);
					}
					else
					{
						lineSink.AddLine(GmpiDrawing::Point(x, y));
					}
					++capturedata;
				}

				lineSink.EndFigure(FigureEnd::Open);
				lineSink.Close();
				//sink.EndFigure();
				//sink.Close();
				g.DrawGeometry(lineGeometry, brush2, penWidth);
			}
		}

		g.PopAxisAlignedClip();

		return gmpi::MP_OK;
	}
};

namespace
{
	auto r = Register<ImpulseResponseGui>::withId(L"SE Impulse Response2");
}
