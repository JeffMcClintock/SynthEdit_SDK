#ifndef AGAIN_DSP_H_INCLUDED
#define AGAIN_DSP_H_INCLUDED

#include "mp_sdk_audio.h"

class AGain : public MpBase2
{
public:
	AGain();
	virtual void onSetPins(void);
	void subProcess( int sampleFrames );

private:
	AudioInPin input1_;
	AudioInPin input2_;
	AudioOutPin output1_;
	AudioOutPin output2_;
	FloatInPin gain_;
};

#endif

