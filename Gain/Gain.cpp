// Copyright 2007 Jeff McClintock

#include "Gain.h"

REGISTER_PLUGIN( Gain, L"SynthEdit Gain example V3" );

Gain::Gain( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, pinInput1 );
	initializePin( 1, pinInput2 );
	initializePin( 2, pinOutput1 );
}

// Process audio.
void Gain::subProcess( int bufferOffset, int sampleFrames )
{
	// assign pointers to your in/output buffers. Each buffer is an array of float samples.
	float* in1  = bufferOffset + pinInput1.getBuffer();
	float* in2  = bufferOffset + pinInput2.getBuffer();
	float* out1 = bufferOffset + pinOutput1.getBuffer();

	for( int s = sampleFrames; s > 0; --s ) // sampleFrames = how many samples to process (can vary). repeat (loop) that many times
	{
		float input1 = *in1;	// get the sample 'POINTED TO' by in1.
		float input2 = *in2;

		// Multiplying the two input's samples together.
		float result = input1 * input2;

		// store the result in the output buffer.
		*out1 = result;

		// increment the pointers (move to next sample in buffers).
		++in1;
		++in2;
		++out1;
	}
}

// One or more inputs updated.  Check pin update flags to determin which ones.
void Gain::onSetPins(void)
{
	/*
	// LEVEL 1 - Simplest way to handle streaming status...
	// Do nothing. That's it.
	*/

	// LEVEL 2 - Determin if output is silent or active, then notify downstream modules.
	//           Downstream modules can then 'sleep' (save CPU) when processing silence.

	// If either input is active, output will be active. ( "||" means "or" ).
	bool OutputIsActive = pinInput1.isStreaming() || pinInput2.isStreaming();

	// Exception...
	// If either input zero, output is silent.
	if( !pinInput1.isStreaming() && pinInput1 == 0.0f )
	{
		OutputIsActive = false;
	}

	if( !pinInput2.isStreaming() && pinInput2 == 0.0f )
	{
		OutputIsActive = false;
	}

	// Transmit new output state to modules 'downstream'.
	pinOutput1.setStreaming( OutputIsActive );

	// Choose which function is used to process audio.
	// You can have one processing method, or several variations (each specialized for a certain condition).
	// For this example only one processing function needed.
	SET_PROCESS( &Gain::subProcess );

	// Normally module will sleep when no inputs or outputs are streaming,
	// however this module is an exception - when the volume on one input pin is zero, module can sleep
	// regardless of the other input.

	// control sleep mode manually.
	setSleep( !OutputIsActive );
}

