#include ".\SignalLogger.h"
#include "PinIterator.h"

using namespace gmpi;

REGISTER_PLUGIN ( SignalLogger, L"SE Signal Logger" );

SignalLogger::SignalLogger( IMpUnknown* host ) : MpBase( host )
	,recordingPosition_(0)
{
}

int32_t SignalLogger::open()
{
	// determine how often to update GUI.
	//int blocksPerUpdate = 15;
	//recordingBufferSize_ = blocksPerUpdate * getBlockSize();

	// Register pins.
	PinIterator it(this);

	pinSignal.assign( it.size(), 0 );
	signalBuffer.assign( it.size(), 0 );

	it.first();
	int idx = 0;
	while( !it.isDone() )
	{
		pinSignal[idx] = new AudioInPin();
		initializePin( (*it)->getUniqueId(), *(pinSignal[idx]) );

		// Buffer for recorded signal.
		signalBuffer[idx] = new float[recordingBufferSize_];

		++it;
		++idx;
	}

	getHost()->sendMessageToGui( -1, 0, 0 ); // reset GUI.

	return MpBase::open();
}

SignalLogger::~SignalLogger()
{
	for( auto it = pinSignal.begin() ; it != pinSignal.end() ; ++it )
	{
		delete (*it);
	}
	for( auto it = signalBuffer.begin() ; it != signalBuffer.end() ; ++it )
	{
		delete [] (*it);
	}
}

void SignalLogger::subProcess( int bufferOffset, int sampleFrames )
{
	int samplefrms = sampleFrames;
	while( samplefrms > 0 )
	{
		int buffer_remain = recordingBufferSize_ - recordingPosition_;
		int todo = min(samplefrms, buffer_remain );

		auto it2 = signalBuffer.begin();
		for( auto it = pinSignal.begin() ; it != pinSignal.end() ; ++it, ++it2 )
		{
			float* signal = bufferOffset + (*it)->getBuffer();
			float* buf = (*it2) + recordingPosition_;
			for( int s = todo; s > 0; --s )
			{
				*buf++ = *signal++;
			}
		}

		recordingPosition_ += todo;

		if( recordingPosition_ >= recordingBufferSize_ )
		{
			recordingPosition_ = 0;
			// send to GUI.
			int idx = 0;
			for( auto it = signalBuffer.begin() ; it != signalBuffer.end() ; ++it )
			{
				getHost()->sendMessageToGui( idx++, sizeof(float) * recordingBufferSize_, *it );
			}
		}

		samplefrms -= todo;
		bufferOffset += todo;
	}
}

void SignalLogger::onSetPins(void)
{
	/*
	// Check which pins are updated.
	if( pinSignalA.isStreaming() )
	{
	}
	if( pinSignalB.isStreaming() )
	{
	}
	if( pinVoiceActive.isUpdated() )
	{
	}
	*/
	// Set processing method.
	SET_PROCESS(&SignalLogger::subProcess);

	// Set sleep mode (optional).
	// setSleep(false);
}

