// Prevent stupid 'min' macro overriding std::min
#include ".\SignalLoggerGui.h"
#include <gdiplus.h>

#undef min
#undef max

using namespace Gdiplus;
using namespace std;

REGISTER_GUI_PLUGIN( SignalLoggerGui, L"SE Signal Logger" );

SignalLoggerGui::SignalLoggerGui( IMpUnknown* host ) : SeGuiWindowsGfxBase(host)
	,capturedFrames_(0)
{
	// initialise pins.
	pinZoomX.initialize( this, 0, static_cast<MpGuiBaseMemberPtr>(&SignalLoggerGui::onSetZoom) );
	pinZoomY.initialize( this, 1, static_cast<MpGuiBaseMemberPtr>(&SignalLoggerGui::onSetZoom) );
	pinPanX.initialize( this, 2, static_cast<MpGuiBaseMemberPtr>(&SignalLoggerGui::onSetZoom) );
	pinPanY.initialize( this, 3, static_cast<MpGuiBaseMemberPtr>(&SignalLoggerGui::onSetZoom) );
}

SignalLoggerGui::~SignalLoggerGui()
{
	Reset();
}

void SignalLoggerGui::Reset()
{
	for( auto it = signaldata.begin(); it != signaldata.end() ; ++it )
	{
		std::vector<captureChunk>& signalarray = (*it);
		for( auto it2 = signalarray.begin(); it2 != signalarray.end() ; ++it2 )
		{
			delete [] (*it2).signal;
			delete [] (*it2).signal_peak;
		}
		signalarray.clear();
	}
}

int32_t SignalLoggerGui::receiveMessageFromAudio( int32_t id, int32_t size, void* messageData )
{
	if( id < 0 )
	{
		Reset();
		return gmpi::MP_OK;
	}

	size_t count = (size_t)size / sizeof(float);

	if( id >= (int) signaldata.size() )
	{
		signaldata.resize(id+1);
		captureCount_ = count;
	}


	signaldata[id].push_back( 0 );
	signaldata[id].back().signal = new float[count];
	memcpy( signaldata[id].back().signal, messageData, size );

	int peakCount = count/peakRate;
	float* peaks = new float[peakCount * 2];
	signaldata[id].back().signal_peak = peaks;
	float* v = signaldata[id].back().signal;
	for( int i = 0 ; i < peakCount*2 ; i+=2 )
	{
		float minimum = FLT_MAX;
		float maximum = -FLT_MAX;
		for(int j = 0 ; j < peakRate ; ++j )
		{
			minimum = std::min(minimum, *v );
			maximum = std::max(maximum, *v );
			++v;
		}

		peaks[i] = minimum;
		peaks[i+1] = maximum;
	}


	if( id == signaldata.size() - 1 )
	{
		capturedFrames_ = signaldata[id].size(); // more frames may arrive during paint(), ignore those til next time.
		invalidateRect();
	}

	return gmpi::MP_OK;
}

int32_t SignalLoggerGui::paint( HDC hDC )
{
	MpRect r = getRect();
	int width = r.right - r.left;
	int height = r.bottom - r.top;
	float y_center = height / 2.0f;

	// Create GDI+ Graphics object to draw on screen.
	Graphics graphics( hDC );
	Pen redPen( Color(0xff, 0xff, 0x00, 0x00), 1.0f);
	Pen greenPen( Color(0xff, 0x00, 0xff, 0x00), 1.0f);
	Pen blackPen( Color(0xff, 0x00, 0x00, 0x00), 1.0f);
	Pen GreyPen( Color(0xff, 0x77, 0x77, 0x77), 1.0f);
	Pen WhitePen( Color(0xff, 0xff, 0xff, 0xff), 1.0f);

	Pen* pens[] = { &redPen, &greenPen, &blackPen, &GreyPen, &WhitePen };
	int currentPenIdx = 0;

	// Background lines.
	for( int v = -10 ; v < 11 ; ++ v )
	{
		float y = y_center - v * height * 0.5f;
		graphics.DrawLine( &GreyPen, 0.0f, y, (float)width, y );
	}

	Pen* currentPen = pens[0];

	SolidBrush blueBrush(Color(255,0,0,255));
	graphics.FillRectangle(&blueBrush,0,0,width,height);
	graphics.SetSmoothingMode( SmoothingModeAntiAlias );

	float y = 0.0f;
	float x_scale = pinZoomX;
	x_scale = max(x_scale,0.25f);
//	x_scale = 1.0f / x_scale;
	float y_scale = pinZoomY * height * 0.5f;
	bool usePeaks = x_scale >= (float) peakRate;
	int peakFrames = captureCount_ / peakRate;
	if( usePeaks )
	{
		x_scale *= (float) peakRate;
	}

	int pan_samples = (int) (pinPanX * 10000.0);
	pan_samples = max( pan_samples, 0 );
	int pan_frames = pan_samples / captureCount_;
	int pan_fine = pan_samples - pan_frames * captureCount_;

	// For each input signal.
	for( auto it = signaldata.begin(); it != signaldata.end() ; ++it )
	{
		std::vector<captureChunk>& signalarray = (*it);

		int frame = capturedFrames_ - 1 - pan_frames;
		int idx = (captureCount_ - 1) - pan_fine;
		int idx_peak = (peakFrames - 1) - pan_fine / peakRate;

		float x = (float) width;
		float prev_y = 0.0f;
		float prev_x =(float) width+1.0f;
		prev_y = y_center;
		while( x >= 0.0f && frame >= 0 )
		{
			if( usePeaks )
			{
				float* signal = signalarray[frame].signal_peak + idx_peak*2;

				for( int i = 0 ; i < idx_peak ; ++i )
				{
					// Adding 0.5 to prevent infintly thin lines not showing.
					float y1 = - 0.5f + y_center - y_scale * ( pinPanY + signal[1] ); // minimum
					float y2 = + 0.5f + y_center - y_scale * ( pinPanY + signal[0] ); // maximum
					graphics.DrawLine( currentPen, x, y1, x, y2 );
					x -= x_scale;
					signal -= 2;
				}
				idx_peak = peakFrames - 1;
			}
			else
			{
				float* signal = signalarray[frame].signal + idx;

				for( int i = 0 ; i < idx ; ++i )
				{
					y = y_center - y_scale * ( pinPanY + *signal-- );
					graphics.DrawLine( currentPen, prev_x, prev_y, x, y );
					prev_y = y;
					prev_x = x;
					x -= x_scale;
				}
				idx = captureCount_ - 1;
			}
			frame--;
		}

		currentPenIdx = (currentPenIdx + 1) % (sizeof(pens) / sizeof(Pen*));
		currentPen = pens[currentPenIdx];
	}

	return gmpi::MP_OK;
}

// handle pin updates.
void SignalLoggerGui::onSetZoom()
{
	invalidateRect();
}

int32_t  SignalLoggerGui::onLButtonDown( UINT flags, POINT point )
{
	setCapture(); // get mouse moves

	previousPoint_ = point;

	return SeGuiWindowsGfxBase::onLButtonDown( flags, point );
}

int32_t  SignalLoggerGui::onLButtonUp( UINT flags, POINT point )
{
	if( getCapture() )
	{
		releaseCapture(); // don't want further mouse move events
	}

	return SeGuiWindowsGfxBase::onLButtonUp( flags, point );
}

int32_t  SignalLoggerGui::onMouseMove( UINT flags, POINT point )
{
	if( !getCapture() )
	{
		return gmpi::MP_OK;
	}

	SIZE offset;
	offset.cx = point.x - previousPoint_.x;
	offset.cy = point.y - previousPoint_.y;

	float x_scale = pinZoomX;
	x_scale = max(x_scale,0.25f);
//	x_scale = 1.0f / x_scale;
	bool usePeaks = x_scale >= (float) peakRate;
	if( usePeaks )
	{
		x_scale *= (float) peakRate;
	}

	float newZoomY = pinZoomY - 0.05f * (float) offset.cy;
	newZoomY = max(newZoomY, 0.01f );
	pinZoomY = newZoomY;

	pinPanX = pinPanX + x_scale * (float) offset.cx;

	previousPoint_ = point;

	return SeGuiWindowsGfxBase::onMouseMove( flags, point );
}

