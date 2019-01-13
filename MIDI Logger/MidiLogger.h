#ifndef MIDILOGGER_H_INCLUDED
#define MIDILOGGER_H_INCLUDED

#include "../se_sdk3/mp_sdk_audio.h"

typedef int64_t timestamp_t;

class MidiLogger : public MpBase
{
public:
	MidiLogger( IMpUnknown* host );
	~MidiLogger();
	virtual void onSetPins(void);
	virtual void onMidiMessage( int pin, unsigned char* midiMessage, int size );
	void subProcess( int bufferOffset, int sampleFrames );

private:
	StringInPin pinFileName;
	MidiInPin pinMidi;

	timestamp_t sampleClock;
	FILE* outputStream;
};

#endif

