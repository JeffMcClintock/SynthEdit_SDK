#include "./NoteExpression.h"
//#include "NoteExpression.xml.h"

REGISTER_PLUGIN2 ( NoteExpression, L"SE Note Expression" );
//REGISTER_XML(NOTEEXPRESSION_XML);

NoteExpression::NoteExpression()
{
	for (int i = 0; i < 8; ++i)
	{
		initializePin(i, inPins[i]);
	}
}

int32_t NoteExpression::open()
{
	MpBase2::open();

	float transitionSmoothing = getSampleRate() * 0.005f; // 5 ms smoothing.
	for (int i = 0; i < 8; ++i)
	{
		int j = i + 8;
		initializePin(j, outPins[i]);
		outPins[i].setCurveType(SmartAudioPin::Curved);
		outPins[i].setTransitionTime(transitionSmoothing);
	}

	return gmpi::MP_OK;
}

void NoteExpression::subProcess( int sampleFrames )
{
	int32_t bufferOffset = getBlockPosition();
	bool canSleep = true;
	for (int i = 0; i < 8; ++i)
	{
		outPins[i].subProcess(bufferOffset, sampleFrames, canSleep);
	}
}

void NoteExpression::onSetPins(void)
{
	for (int i = 0; i < 8; ++i)
	{
		if (inPins[i].isUpdated())
		{
			outPins[i] = 0.1f * inPins[i];
		}
	}

	// Set processing method.
	SET_PROCESS2(&NoteExpression::subProcess);

	// Set sleep mode (optional).
	// setSleep(false);
}

