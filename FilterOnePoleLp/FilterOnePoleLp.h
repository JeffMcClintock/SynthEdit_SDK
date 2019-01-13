#ifndef FILTERONEPOLELP_H_INCLUDED
#define FILTERONEPOLELP_H_INCLUDED

#include "mp_sdk_audio.h"

class FilterOnePoleLp : public MpBase
{
public:
	FilterOnePoleLp( IMpUnknown* host );
	void subProcess( int bufferOffset, int sampleFrames );
	void subWaitUntilSettled( int bufferOffset, int sampleFrames );
	void subProcessSettled( int bufferOffset, int sampleFrames );
	virtual void onSetPins(void);
	void DoPeriodicCheck(int blockPosition);

private:
	AudioInPin pinSignal;
	AudioInPin pinPitch;
	AudioOutPin pinOutput;

	float y1n;
	float l;
	int periodicCheckCount_;
};

#endif

