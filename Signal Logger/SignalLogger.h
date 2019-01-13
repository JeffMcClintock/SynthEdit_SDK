#ifndef SIGNALLOGGER_H_INCLUDED
#define SIGNALLOGGER_H_INCLUDED

#include <vector>
#include "mp_sdk_audio.h"

class SignalLogger : public MpBase
{
public:
	SignalLogger( IMpUnknown* host );
	~SignalLogger();

	virtual int32_t MP_STDCALL open();
	void subProcess( int bufferOffset, int sampleFrames );
	virtual void onSetPins(void);

private:
	std::vector<AudioInPin*> pinSignal;
	std::vector<float*> signalBuffer;
	int recordingPosition_;
	static const int recordingBufferSize_ = 512;
};

#endif

