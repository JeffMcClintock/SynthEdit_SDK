#include "./AGain.h"

REGISTER_PLUGIN2( AGain, L"Gain" );

AGain::AGain() : MpBase2( )
{
	// Register pins.
	initializePin( input1_ );
	initializePin( input2_ );
	initializePin( output1_ );
	initializePin( output2_ );
	initializePin( gain_ );
}

void AGain::onSetPins(void)
{
	// Specify which function is used to process audio.
	SET_PROCESS2( &AGain::subProcess );
}

void AGain::subProcess( int sampleFrames )
{
	// get parameter value.
	float gain = gain_;

	// get pointers to in/output buffers.
	auto input1	 = getBuffer(input1_);
	auto input2	 = getBuffer(input2_);
	auto output1 = getBuffer(output1_);
	auto output2 = getBuffer(output2_);

	// Apply audio processing.
	while( --sampleFrames >= 0 )
	{
		*output1++ = gain * *input1++;
		*output2++ = gain * *input2++;
	}
}

