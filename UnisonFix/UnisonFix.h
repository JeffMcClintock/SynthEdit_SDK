#ifndef UNISONFIX_H_INCLUDED
#define UNISONFIX_H_INCLUDED

#include "mp_sdk_audio.h"
#include "hasMidiTuning.h"

class UnisonFix : public MpBase, public hasMidiTuning
{
public:
	UnisonFix( IMpUnknown* host );
	virtual void onMidiMessage(int pin, unsigned char* midiMessage, int size); // size < 4 for short msg, or > 4 for sysex
	virtual void OnKeyTuningChanged(int p_clock, int MidiNoteNumber, int tune) override;

private:
	MidiInPin pinMIDIIn;
	MidiOutPin pinMIDIOut;

	static const int VirtualVoiceCount = 128;
	static const int MidiChannelCount = 16;
	static const int MidiKeyCount = 128;
	int voiceReleaseOrder[VirtualVoiceCount];
	int voiceChannel[VirtualVoiceCount];
	int voiceKey[VirtualVoiceCount];
	char keyMap[MidiChannelCount][MidiKeyCount];
	int nextReleaseIndex;
};

#endif

