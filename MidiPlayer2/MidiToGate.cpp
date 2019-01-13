#include "../se_sdk3/mp_sdk_audio.h"
#include "../se_sdk3/mp_midi.h"
#include <array>

using namespace gmpi;

class MidiToGate : public MpBase2
{
	MidiInPin pinMIDIIn;
	BoolOutPin pinGate;
	BoolInPin pinRetrigger;

	std::array<bool, 128> keyStates;
	int retriggerCounter_ = 0;

public:
	MidiToGate()
	{
		initializePin( pinMIDIIn );
		initializePin( pinRetrigger );
		initializePin( pinGate );

		keyStates.fill(false);
	}

	void subProcess( int sampleFrames )
	{
		if (sampleFrames > retriggerCounter_)
		{
			pinGate.setValue(true, getBlockPosition() + retriggerCounter_);
			setSleep(true);
			setSubProcess(&MidiToGate::subProcessNothing);
		}

		retriggerCounter_ -= sampleFrames;
	}

	void onMidiMessage(int pin, unsigned char* midiMessage, int size)
	{
		int stat, b2, b3, midiChannel; // 3 bytes of MIDI message

		midiChannel = midiMessage[0] & 0x0f;

		stat = midiMessage[0] & 0xf0;
		b2 = midiMessage[1];
		b3 = midiMessage[2];

		// Note offs can be note_on vel=0
		if (b3 == 0 && stat == GmpiMidi::MIDI_NoteOn)
		{
			stat = GmpiMidi::MIDI_NoteOff;
		}

		switch (stat)
		{
		case GmpiMidi::MIDI_NoteOn:
		{
			keyStates[b2] = true;

			if (pinGate && pinRetrigger)
			{
				pinGate = false;
				retriggerCounter_ = 3;
				setSleep(false);
				setSubProcess(&MidiToGate::subProcess);
			}
			else
			{
				pinGate = true;
			}
		}
		break;

		case GmpiMidi::MIDI_NoteOff:
		{
			bool g = false;
			keyStates[b2] = false;
			for (auto keyState : keyStates)
			{
				if (keyState)
				{
					g = true;
					break;
				}
			}
			if (!g)
			{
				pinGate = false;
			}
		}
		break;

		default:
			break;
		}
	}
};

namespace
{
	auto r = Register<MidiToGate>::withId(L"SE MIDItoGate");
}
