#include "./Converters.h"
#include "../shared/xp_simd.h"

typedef SimpleConverter<int, bool> IntToBool;
typedef SimpleConverter<int, float> IntToFloat;
typedef SimpleConverter<int, std::wstring> IntToText;

typedef SimpleConverter<float, int> FloatToInt;
typedef SimpleConverter<float, bool> FloatToBool;
typedef SimpleConverter<float, std::wstring> FloatToText;

typedef SimpleConverter<bool, int> BoolToInt;
typedef SimpleConverter<bool, float> BoolToFloat;
typedef SimpleConverter<bool, std::wstring> BoolToText;

typedef SimpleConverter<std::wstring, int> TextToInt;
typedef SimpleConverter<std::wstring, float> TextToFloat;
typedef SimpleConverter<std::wstring, bool> TextToBool;

typedef SimpleConverter<int, int> ListToInt;


REGISTER_PLUGIN( IntToBool, L"SE IntToBool" );
REGISTER_PLUGIN( IntToFloat, L"SE IntToFloat" );
REGISTER_PLUGIN( IntToText, L"SE IntToText" );

REGISTER_PLUGIN( FloatToInt, L"SE FloatToInt" );
REGISTER_PLUGIN( FloatToBool, L"SE FloatToBool" );
REGISTER_PLUGIN( FloatToText, L"SE FloatToText" );

REGISTER_PLUGIN( BoolToInt, L"SE BoolToInt" );
REGISTER_PLUGIN( BoolToFloat, L"SE BoolToFloat" );
REGISTER_PLUGIN( BoolToText, L"SE BoolToText" );

REGISTER_PLUGIN( TextToInt, L"SE TextToInt" );
REGISTER_PLUGIN( TextToFloat, L"SE TextToFloat" );
REGISTER_PLUGIN( TextToBool, L"SE TextToBool" );

REGISTER_PLUGIN( ListToInt, L"SE ListToInt" );



class VoltsToBool : public MpBase2
{
	AudioInPin pinVoltsIn;
	BoolOutPin pinBoolOut;
	bool state;

public:
	VoltsToBool() :
		state(false)
	{
		initializePin(pinVoltsIn);
		initializePin(pinBoolOut);
	}

	void subProcess(int sampleFrames)
	{
		const float* voltsIn = getBuffer(pinVoltsIn);

		for (int s = sampleFrames; s > 0; --s)
		{
			if (state != (*voltsIn > 0.0f))
			{
				state = (*voltsIn > 0.0f);
				auto blockPos = getBlockPosition() + sampleFrames - s;
				pinBoolOut.setValue(state, blockPos);
			}

			// Increment buffer pointers.
			++voltsIn;
		}
	}

	virtual void onSetPins(void) override
	{
		setSubProcess(&VoltsToBool::subProcess);
	}
};

REGISTER_PLUGIN2(VoltsToBool, L"SE VoltsToBool");

class BoolToVolts : public MpBase2
{
	BoolInPin pinBoolIn;
	AudioOutPin pinVoltsOut;

public:
	BoolToVolts()
	{
		initializePin(pinBoolIn);
		initializePin(pinVoltsOut);
	}

	void subProcess(int sampleFrames)
	{
		auto output = getBuffer(pinVoltsOut);

		float value = pinBoolIn ? 1.0f : 0.0f;

		for (int s = sampleFrames; s > 0; --s)
		{
			*output++ = value;
		}
	}

	virtual void onSetPins(void) override
	{
		setSubProcess(&BoolToVolts::subProcess);
		pinVoltsOut.setStreaming(false);
	}
};

REGISTER_PLUGIN2(BoolToVolts, L"SE BoolToVolts");

class VoltsToInt : public MpBase2
{
	AudioInPin pinVoltsIn;
	IntOutPin pinIntOut;
	int state;

public:
	VoltsToInt() :
		state(0)
	{
		initializePin(pinVoltsIn);
		initializePin(pinIntOut);
	}

	void subProcess(int sampleFrames)
	{
		const float* voltsIn = getBuffer(pinVoltsIn);

		for (int s = sampleFrames; s > 0; --s)
		{
			int v = FastRealToIntFloor(*voltsIn * 10.0f + 0.5f);
			if (state != v)
			{
				state = v;
				auto blockPos = getBlockPosition() + sampleFrames - s;
				pinIntOut.setValue(state, blockPos);
			}

			// Increment buffer pointers.
			++voltsIn;
		}
	}

	virtual void onSetPins(void) override
	{
		setSubProcess(&VoltsToInt::subProcess);
	}
};

REGISTER_PLUGIN2(VoltsToInt, L"SE VoltsToInt");

class IntToVolts : public MpBase2
{
	IntInPin pinIntIn;
	AudioOutPin pinVoltsOut;

public:
	IntToVolts()
	{
		initializePin(pinIntIn);
		initializePin(pinVoltsOut);
	}

	void subProcess(int sampleFrames)
	{
		auto output = getBuffer(pinVoltsOut);

		float value = pinIntIn * 0.1f;

		for (int s = sampleFrames; s > 0; --s)
		{
			*output++ = value;
		}
	}

	virtual void onSetPins(void) override
	{
		setSubProcess(&IntToVolts::subProcess);
		pinVoltsOut.setStreaming(false);
	}
};

REGISTER_PLUGIN2(IntToVolts, L"SE IntToVolts");