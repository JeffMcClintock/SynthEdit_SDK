#include "./Slider.h"

REGISTER_PLUGIN2 ( Slider, L"SE Slider" );

Slider::Slider()
{
	// Register pins.
	initializePin( pinValueIn);
	initializePin( pinValueOut );
}

void Slider::subProcess( int sampleFrames )
{
	bool canSleep = true;
	pinValueOut.subProcess(getBlockPosition(), sampleFrames, canSleep);
}

void Slider::onSetPins(void)
{
	pinValueOut = 0.1f * pinValueIn; // 1 V = 0.1 audio.

	// Set processing method.
	SET_PROCESS2(&Slider::subProcess);
}

