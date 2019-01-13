#pragma once

#include <algorithm>
#include <math.h>
#include "../se_sdk3/mp_sdk_audio.h"

/*
#include "../shared/FilterBase.h"
*/

/*
This class implements advanced 'sleep' behaviour for filters or any module that takes some time to settle after the input signal goes quiet.
This base class monitors the output signal, only when the output has settled into a steady-state does the module go to sleep.

USEAGE:

Derive your module from class FilterBase

	class MyFilter : public FilterBase
	{
	// etc
	}

Implement these member functions

	// Support for FilterBase

	// Optional: This is called periodically to allow you to check if your filter has 'crashed' and you need to reset it.
	// You can leave it empty.
	virtual void StabilityCheck() override
	{
	}

	// This is called to determin when your filter is settling. Typically you need to check if all your input pins are quiet (not streaming).
	virtual bool isFilterSettling() override
	{
		return !pinSignal.isStreaming(); // <-example code. Place your code here.
	}

	// This allows the base class to monitor the filter's output signal, provide an audio output pin.
	virtual AudioOutPin& getOutputPin() override
	{
		return pinOutput; // <-example code. Place your code here.
	}

Call initSettling() as the last thing in your onSetPins() method. It will check if filter is setlling and if so commence monitoring the output signal.

	void MyFilter::onSetPins(void)
	{

		// ... usual stuff first.

		initSettling(); // must be last.
	}

In you processing member function, call doStabilityCheck() to implement the periodic StabilityCheck() function.
It calls your StabilityCheck() member only one time in 50, to avoid wasting CPU. This must be the first thing in the function.

	void subProcess(int sampleFrames)
	{
		doStabilityCheck();

		// ...etc

*/

class FilterBase : public MpBase2
{
protected:
	SubProcess_ptr2 actualSubProcess;

	int32_t stabilityCheckCounter = 0;
	float static_output;

	static const int historyCount = 32;
	int historyIdx = 0;

public:
	FilterBase()
	{}

	virtual int32_t MP_STDCALL open() override
	{
		// randomise stabilityCheckCounter to avoid CPU spikes.
		getHost()->getHandle(stabilityCheckCounter);
		stabilityCheckCounter &= 63;

		return MpBase2::open();
	}

	virtual bool isFilterSettling() = 0;
	virtual void OnFilterSettled()
	{
		setSubProcess(&FilterBase::subProcessStatic);

		const int blockPosition = getBlockPosition(); // assuming StabilityCheck() always called at start of block.
		getOutputPin().setStreaming(false, blockPosition);
	}
	virtual AudioOutPin& getOutputPin() = 0;

	// This provides a counter to check the filter periodically to see it it has 'crashed'.
	inline void doStabilityCheck()
	{
		if (--stabilityCheckCounter < 0)
		{
			StabilityCheck();
			stabilityCheckCounter = 50;
		}
	}

	// override this to check if your filter memory contains invalid data.
	virtual void StabilityCheck() {}

	void subProcessSettling(int sampleFrames)
	{
		if (historyIdx <= 0)
		{
			static_output = getBuffer(getOutputPin())[0];
			setSubProcess(actualSubProcess); // safely measure to prevent accidental recursion.
			OnFilterSettled();
			return (this->*(getSubProcess()))(sampleFrames); // call subProcessStatic().
		}

		(this->*(actualSubProcess))(sampleFrames);

		int todo = (std::min) (historyIdx, sampleFrames);
		auto o = getBuffer(getOutputPin()) + sampleFrames - todo;
		for (int i = 0; i < todo; ++i)
		{
			const float INSIGNIFICANT_SAMPLE = 0.000001f;
			float energy = fabs(static_output - *o);
			if (energy > INSIGNIFICANT_SAMPLE)
			{
				// filter still not settled.
				historyIdx = historyCount;
				static_output = *o;
			}
			--historyIdx;
			++o;
		}
	}

	// This assumes your filter has only one output, override OnFilterSettled() to use your own member function if needed.
	void subProcessStatic(int sampleFrames)
	{
		auto output = getBuffer(getOutputPin());

		for (int s = sampleFrames; s > 0; s--)
		{
			*output++ = static_output;
		}
	}

	void initSettling(void)
	{
		if (isFilterSettling())
		{
			historyIdx = historyCount;
			actualSubProcess = getSubProcess();
			static_output = -10000; // unlikely value to trigger countdown.
			setSubProcess(&FilterBase::subProcessSettling);
		}
	}
};


