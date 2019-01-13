#ifndef SIGNALLOGGERGUI_H_INCLUDED
#define SIGNALLOGGERGUI_H_INCLUDED

#include <vector>
#include "MP_SDK_GUI.h"

struct captureChunk
{
	captureChunk(int nothing) : signal(0), signal_peak(0){};
	float* signal;
	float* signal_peak;
};

class SignalLoggerGui : public SeGuiWindowsGfxBase
{
public:
	SignalLoggerGui( IMpUnknown* host );
	~SignalLoggerGui();

	// overrides
	virtual int32_t MP_STDCALL paint( HDC hDC );
	virtual int32_t MP_STDCALL receiveMessageFromAudio( int32_t id, int32_t size, void* messageData );
	virtual int32_t MP_STDCALL onLButtonDown( UINT flags, POINT point );
	virtual int32_t MP_STDCALL onLButtonUp( UINT flags, POINT point );
	virtual int32_t MP_STDCALL onMouseMove( UINT flags, POINT point );

	void onSetZoom();

private:
	void Reset();
 	FloatGuiPin pinZoomX;
 	FloatGuiPin pinZoomY;
 	FloatGuiPin pinPanX;
 	FloatGuiPin pinPanY;

	std::vector< std::vector<captureChunk> > signaldata;
	int captureCount_;
	int capturedFrames_;

	static const int peakRate = 16;
	POINT previousPoint_;
};

#endif


