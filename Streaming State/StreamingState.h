#ifndef STREAMINGSTATE_H_INCLUDED
#define STREAMINGSTATE_H_INCLUDED

#include "mp_sdk_audio.h"

class StreamingState : public MpBase
{
public:
	StreamingState( IMpUnknown* host );
	void subProcess( int bufferOffset, int sampleFrames );
	virtual void onSetPins(void);

private:
	AudioInPin pinSignalIn;
	AudioOutPin pinSignalOut;
	float output_;
};

#endif

