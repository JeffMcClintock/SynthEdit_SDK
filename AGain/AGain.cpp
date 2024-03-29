#include "mp_sdk_audio.h"

using namespace gmpi;
using namespace GmpiSdk;

class AGain final : public MpBase2
{
	AudioInPin pinInput1;
	AudioInPin pinInput2;
	AudioOutPin pinOutput1;
	AudioOutPin pinOutput2;
	FloatInPin pinGain;

public:
	AGain()
	{
		initializePin(pinInput1);
		initializePin(pinInput2);
		initializePin(pinOutput1);
		initializePin(pinOutput2);
		initializePin(pinGain);
}

	void subProcess(int sampleFrames)
{
	// get parameter value.
		const float gain = pinGain;

	// get pointers to in/output buffers.
		auto input1 = getBuffer(pinInput1);
		auto input2 = getBuffer(pinInput2);
		auto output1 = getBuffer(pinOutput1);
		auto output2 = getBuffer(pinOutput2);

	// Apply audio processing.
	while( --sampleFrames >= 0 )
	{
		*output1++ = gain * *input1++;
		*output2++ = gain * *input2++;
	}
}

	void onSetPins() override
	{
		// Specify which function is used to process audio.
		setSubProcess(&AGain::subProcess);
	}
};

namespace
{
	auto r = Register<AGain>::withId(L"JM Gain");
}
