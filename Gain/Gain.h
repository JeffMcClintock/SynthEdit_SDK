// Copyright 2007 Jeff McClintock

#ifndef GAIN_H_INCLUDED
#define GAIN_H_INCLUDED

#include "../se_sdk3/mp_sdk_audio.h"

class Gain: public MpBase
{
public:
	Gain(IMpUnknown* host);
	void subProcess(int bufferOffset, int sampleFrames);
	virtual void onSetPins(void);

private:
	AudioInPin pinInput1;
	AudioInPin pinInput2;
	AudioOutPin pinOutput1;
};

#endif
