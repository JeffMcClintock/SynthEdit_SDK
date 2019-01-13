#include "./StreamingState.h"

REGISTER_PLUGIN ( StreamingState, L"SE Streaming State" );

StreamingState::StreamingState( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, pinSignalIn );
	initializePin( 1, pinSignalOut );
}

void StreamingState::subProcess( int bufferOffset, int sampleFrames )
{
	float* signalOut = bufferOffset + pinSignalOut.getBuffer();
	float state = (float) pinSignalIn.isStreaming();

	for( int s = sampleFrames; s > 0; --s )
	{
		*signalOut = output_;
		++signalOut;

		if( output_ != state )
		{
			output_ = state;
			pinSignalOut.setStreaming(false, bufferOffset + sampleFrames - s );
		}
	}
}

void StreamingState::onSetPins(void)
{
	// Set state of output audio pins.
	pinSignalOut.setStreaming(false);

	if( pinSignalIn.isUpdated() && !pinSignalIn.isStreaming() )
	{
		output_ = 0.5f;
	}

	// Set processing method.
	SET_PROCESS(&StreamingState::subProcess);
}

