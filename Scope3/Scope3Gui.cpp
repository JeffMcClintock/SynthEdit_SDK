// prevent MS CPP - 'swprintf' was declared deprecated warning
#if defined(_MSC_VER)
  #define _CRT_SECURE_NO_DEPRECATE 
  #pragma warning(disable : 4996)
#endif

#include "Scope3Gui.h"

REGISTER_GUI_PLUGIN( Scope3Gui, L"SynthEdit Scope3" );

Scope3Gui::Scope3Gui( IMpUnknown* host ) : SeGuiCompositedGfxBase( host )
,newestVoice_( 0 )
{
	initializePin( 0, pinSamplesA, static_cast<MpGuiBaseMemberIndexedPtr>( &Scope3Gui::onValueChanged ) );
	initializePin( 1, pinSamplesB, static_cast<MpGuiBaseMemberIndexedPtr>( &Scope3Gui::onValueChanged ) );
	initializePin( 3, pinGates,    static_cast<MpGuiBaseMemberIndexedPtr>( &Scope3Gui::onVoicesActiveChanged ) );
	initializePin( 4, pinPolyMode, static_cast<MpGuiBaseMemberPtr>( &Scope3Gui::onPolyModeChanged ) );

	for( int i = 0 ; i < MP_VOICE_COUNT ; ++i )
	{
		VoiceLastUpdated[i] = 0;
	}
}

void Scope3Gui::onValueChanged( int voiceId )
{
	VoiceLastUpdated[voiceId] = timeGetTime();

	invalidateRect();
	StartTimer();
}

void Scope3Gui::onVoicesActiveChanged( int voiceId )
{
	if( pinPolyMode == true && pinGates.getValue( voiceId ) )
	{
		newestVoice_ = voiceId;
	}
}

void Scope3Gui::onPolyModeChanged()
{
	if( pinPolyMode == false )
	{
		newestVoice_ = 0; // monophonic mode.
	}
}

// start regular timer calls.
int32_t MP_STDCALL Scope3Gui::openWindow( void )
{
	// animationRate changed.  Calculate frame duration in ms.
	const int animationRate = 100; // ms
	int interval = (int) ( 1000.0f / animationRate );

	SetTimerIntervalMs( interval );

//	StartTimer();

	return gmpi::MP_OK;
}

// stop timer calls.
int32_t MP_STDCALL Scope3Gui::closeWindow( void )
{
	StopTimer();

	return gmpi::MP_OK;
}

bool Scope3Gui::OnTimer()
{
	invalidateRect();
	return true;
}

int32_t Scope3Gui::paint(HDC hDC)
{
	DWORD showUpdatesAfter = timeGetTime() - 500; // show any trace updated in last 0.5 seconds. 

	MpRect r = getRect();
	int width = r.right - r.left;
	int height = r.bottom - r.top;

	HFONT font_handle;
	getGuiHost()->getFontInfo( L"tty", fontInfo_, font_handle );

	float scale = height / 2.15f;
	int mid_x = width/2;
	int mid_y = height/2;

	if( fontInfo_.colorBackground >= 0 ) // -1 indicates transparent background
	{
		// Fill in solid background black
		HBRUSH background_brush = CreateSolidBrush(fontInfo_.colorBackground);

		RECT r;
		r.top = r.left = 0;
		r.right = width + 1;
		r.bottom = height + 1;
		FillRect(hDC, &r, background_brush);

		// cleanup objects
		DeleteObject(background_brush);
	}

	// create a green pen
	int darked_col = (fontInfo_.color >> 1) &0x7f7f7f;
	HPEN pen = CreatePen(PS_SOLID, 1, darked_col); // dark green

	// 'select' it
	HGDIOBJ old_pen = SelectObject(hDC, pen);

	// BACKGROUND LINES
	// horizontal line
	MoveToEx(hDC, 0, mid_y, 0);
	LineTo(hDC, width, mid_y);
	// vertical line
	MoveToEx(hDC, mid_x, 0, 0);
	LineTo(hDC, mid_x, height);

	// voltage ticks
	int tick_width = 2;
	int step = 1;
	if(height < 50)
		step = 4;

	for( int v = -10 ; v < 11 ; v += step )
	{
		float y = v * scale / 10.f;
		
		if(v % 5 == 0)
			tick_width = 4;
		else
			tick_width = 2;

		MoveToEx(hDC, mid_x - tick_width,mid_y + (int)y, 0);
		LineTo(hDC, mid_x + tick_width,mid_y + (int)y);
	}

	// labels
	if( height > 30 )
	{
		int fontHeight = fontInfo_.fontHeight;

		HGDIOBJ old_font = SelectObject( hDC, font_handle );

		SetTextColor( hDC, fontInfo_.color );
		SetBkMode( hDC, TRANSPARENT );
		SetTextAlign( hDC, TA_LEFT );

		for( int v = -10 ; v < 11 ; v += 5 )
		{
			wchar_t txt[10];
			float y = v * scale / 10.f;
			swprintf(txt, L"%2.0f", (float) v);

			TextOut(hDC, mid_x + tick_width, mid_y - (int) y - fontHeight/2, txt, (int) wcslen(txt));
		}

		SelectObject(hDC, old_font);
	}

	// trace
	HPEN newPen = CreatePen(PS_SOLID, 1, RGB(150,150,0)); // dark yellow
	SelectObject(hDC, newPen); // will deselect current pen.
	DeleteObject(pen);
	pen = newPen;

	bool voicesStillActive = false;

	// non-main voices drawn dull.

	if( pinPolyMode == true )
	{
		// 'B' trace.
		for(int voice = 0 ; voice < 128 ; ++voice )
		{
			if( pinGates.getValue(voice) || VoiceLastUpdated[voice] > showUpdatesAfter ) 
			{
				voicesStillActive = true;

				if( voice != newestVoice_ ) 
				{
					if( pinSamplesB.rawSize(voice) == sizeof(float) * SCOPE_BUFFER_SIZE )
					{
						float* capturedata = (float*) pinSamplesB.rawData(voice);
						MoveToEx( hDC, 0, mid_y - (int) (*capturedata * scale), 0);
						++capturedata;

						for(int i = 1 ; i < SCOPE_BUFFER_SIZE ; i++)
						{
							int x = (i * width) / SCOPE_BUFFER_SIZE;
							LineTo( hDC, x, mid_y - (int) (*capturedata * scale) );
							++capturedata;
						}

					}
				}
			}
		}

		// 'A' trace.
		newPen = CreatePen(PS_SOLID, 1, RGB(0,160,0)); // dark green
		SelectObject(hDC, newPen); // will deselect current pen.
		DeleteObject(pen);
		pen = newPen;

		for(int voice = 0 ; voice < 128 ; ++voice )
		{
			if( pinGates.getValue(voice) || VoiceLastUpdated[voice] > showUpdatesAfter ) 
			{
				voicesStillActive = true;

				if( voice != newestVoice_ )
				{
					if( pinSamplesA.rawSize(voice) == sizeof(float) * SCOPE_BUFFER_SIZE )
					{
						float* capturedata = (float*) pinSamplesA.rawData(voice);
						MoveToEx( hDC, 0, mid_y - (int) (*capturedata * scale), 0);
						++capturedata;

						for(int i = 1 ; i < SCOPE_BUFFER_SIZE ; i++)
						{
							int x = (i * width) / SCOPE_BUFFER_SIZE;
							LineTo( hDC, x, mid_y - (int) (*capturedata * scale) );
							++capturedata;
						}
					}
				}
			}
		}
	}

	// main trace drawn bright.
	// Note: Only works when MIDI connected to Patch-Automator. Else Gate host controls not active.
	if( ( newestVoice_ >= 0 && (pinGates.getValue(newestVoice_) || VoiceLastUpdated[newestVoice_] > showUpdatesAfter )) || pinPolyMode == false ) 
	{
		if( pinSamplesB.rawSize(newestVoice_) == sizeof(float) * SCOPE_BUFFER_SIZE )
		{
			newPen = CreatePen(PS_SOLID, 1, RGB(250,250,0)); // bright green
			SelectObject(hDC, newPen); // will deselect current pen.
			DeleteObject(pen);
			pen = newPen;

			float* capturedata = (float*) pinSamplesB.rawData(newestVoice_);
			MoveToEx( hDC, 0, mid_y - (int) (*capturedata * scale), 0);
			++capturedata;

			for(int i = 1 ; i < SCOPE_BUFFER_SIZE ; i++)
			{
				int x = (i * width) / SCOPE_BUFFER_SIZE;
				LineTo( hDC, x, mid_y - (int) (*capturedata * scale) );
				++capturedata;
			}
		}

		if( pinSamplesA.rawSize(newestVoice_) == sizeof(float) * SCOPE_BUFFER_SIZE )
		{
			newPen = CreatePen(PS_SOLID, 1, RGB(0,255,0)); // bright green
			SelectObject(hDC, newPen); // will deselect current pen.
			DeleteObject(pen);
			pen = newPen;
			float* capturedata = (float*) pinSamplesA.rawData(newestVoice_);
			MoveToEx( hDC, 0, mid_y - (int) (*capturedata * scale), 0);
			++capturedata;

			for(int i = 1 ; i < SCOPE_BUFFER_SIZE ; i++)
			{
				int x = (i * width) / SCOPE_BUFFER_SIZE;
				LineTo( hDC, x, mid_y - (int) (*capturedata * scale) );
				++capturedata;
			}
		}
	}

	// cleanup
	SelectObject(hDC, old_pen);
	DeleteObject(pen);

	if( !voicesStillActive )
	{
		StopTimer();
	}

	return gmpi::MP_OK;
}

int32_t Scope3Gui::measure(MpSize availableSize, MpSize &returnDesiredSize)
{
	const int prefferedSize = 100;
	const int minSize = 15;

	returnDesiredSize.x = availableSize.x;
	returnDesiredSize.y = availableSize.y;
	if(returnDesiredSize.x > prefferedSize)
	{
		returnDesiredSize.x = prefferedSize;
	}
	else
	{
		if(returnDesiredSize.x < minSize)
		{
			returnDesiredSize.x = minSize;
		}
	}
	if(returnDesiredSize.y > prefferedSize)
	{
		returnDesiredSize.y = prefferedSize;
	}
	else
	{
		if(returnDesiredSize.y < minSize)
		{
			returnDesiredSize.y = minSize;
		}
	}
	return gmpi::MP_OK;
};
