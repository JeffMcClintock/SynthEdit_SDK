#define _USE_MATH_DEFINES
#include <math.h>
#include ".\FilterOnePoleLp.h"

REGISTER_PLUGIN ( FilterOnePoleLp, L"SE Filter One Pole LP" );

FilterOnePoleLp::FilterOnePoleLp( IMpUnknown* host ) : MpBase( host )
	,y1n(0.0f)
	,periodicCheckCount_( 1 )
{
	// Register pins.
	initializePin( 0, pinSignal );
	initializePin( 1, pinPitch );
	initializePin( 2, pinOutput );
}

// This is the filter code.
void FilterOnePoleLp::subProcess( int bufferOffset, int sampleFrames )
{
	// Periodically check the filter's stability etc.
	if( periodicCheckCount_-- < 0 )
	{
		DoPeriodicCheck( bufferOffset );
	}

	// get pointers to in/output buffers.
	float* signal	= bufferOffset + pinSignal.getBuffer();
	float* pitch	= bufferOffset + pinPitch.getBuffer();
	float* output	= bufferOffset + pinOutput.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{
		// Calculate the co-efs based on the pitch.
		float freq_hz = 10.f * *pitch++;
		freq_hz = min( freq_hz, getSampleRate() * 0.495f ); // limit to 1/2 sample rate
		l = expf( -( 2.0f * (float) M_PI ) * freq_hz / getSampleRate() );

		// Apply the filter to the input signal.
		float xn = *signal++;
		y1n = xn + l * ( y1n - xn );
		*output++ = y1n;
	}
}

// This alternate method runs when the input signal is silent.
// It monitors the state of the filter, once it has settled down to silence we switch to 'subProcessSettled'.
void FilterOnePoleLp::subWaitUntilSettled( int bufferOffset, int sampleFrames )
{
	int blockPosition = bufferOffset + sampleFrames - 1;
	if(blockPosition < 0)
	{
		blockPosition += getBlockSize();
	}

	int previousBlockPos = blockPosition - 1;
	if(previousBlockPos < 0)
	{
		previousBlockPos += getBlockSize();
	}

	float input = pinSignal.getValue(blockPosition);

	// Rough estimate of energy in filter. Filter will sleep when it's settled down.
	float energy = fabs(input-y1n);

	// Alternatly stop filter when roundoff error results in output getting stuck at the same value.
	float PreviousOutput = pinOutput.getBuffer()[previousBlockPos];

	const float INSIGNIFICANT_SAMPLE = 0.000001f;
	if( PreviousOutput == y1n || energy < INSIGNIFICANT_SAMPLE ) // y1n = current output
	{
		// Filter has settled. Output silence.
		SET_PROCESS(&FilterOnePoleLp::subProcessSettled);

		// clamp output to exact input value.
		y1n = input;

		// Set output pin to 'not-streaming'.
		pinOutput.setStreaming( false, blockPosition );

		// Enable automatic sleep mode. Module will sleep once output buffers are filled with silence.
		setSleep(true);

		// Output silence.
		subProcessSettled( bufferOffset, sampleFrames );
	}
	else
	{
		// Filter hasn't settled yet. Process the filter some more.
		subProcess( bufferOffset, sampleFrames );
	}
}

void FilterOnePoleLp::subProcessSettled( int bufferOffset, int sampleFrames )
{
	float* output = bufferOffset + pinOutput.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{
		*output++ = y1n;
	}
}

void FilterOnePoleLp::onSetPins(void)
{
	// Set state of output audio pins.
	pinOutput.setStreaming(true);

	// As this plugin has memory (is resonant) prevent automatic sleep mode, even when the input is silent.
	setSleep(false);

	// Set processing method.
	if( pinSignal.isStreaming() )
	{
		SET_PROCESS(&FilterOnePoleLp::subProcess);
	}
	else
	{
		SET_PROCESS(&FilterOnePoleLp::subWaitUntilSettled);
	}
}

// Filter can be unstable and get stuck in a invalid state. This method corrects such cases.
void FilterOnePoleLp::DoPeriodicCheck( int blockPosition )
{
	periodicCheckCount_ = 10; // Reset counter. Approx every 1/4 second check filter stability etc.

	// Correct for numeric overflow.
	if( !_finite(y1n) )
	{
		y1n = 0.f;
	}
}
