#pragma once

#include "mp_sdk_gui.h"
#include "Scope3.h"
#include "TimerManager.h"

class Scope3Gui :
	public SeGuiCompositedGfxBase, public TimerClient
{
public:
	Scope3Gui(IMpUnknown* host);

	// IMpGraphicsWinGdi overrides.
	virtual int32_t MP_STDCALL measure( MpSize availableSize, MpSize& returnDesiredSize );

	// SeGuiCompositedGfxBase overrides.
	virtual int32_t MP_STDCALL openWindow( void );
	virtual int32_t MP_STDCALL closeWindow( void );
	virtual int32_t MP_STDCALL paint(HDC hDC);

	void onValueChanged( int voiceId );
	void onVoicesActiveChanged( int voiceId );
	void onPolyModeChanged();

	// TimerClient overide.
	virtual bool OnTimer() override;

	BlobArrayGuiPin pinSamplesA;
	BlobArrayGuiPin pinSamplesB;
	FloatArrayGuiPin pinGates;
	BoolGuiPin pinPolyMode;

private:
	MpFontInfo fontInfo_;
	int newestVoice_;
	DWORD VoiceLastUpdated[MP_VOICE_COUNT];
};
