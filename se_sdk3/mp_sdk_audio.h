// Copyright 2006 Jeff McClintock

#ifndef MP_SDK_AUDIO_H_INCLUDED
#define MP_SDK_AUDIO_H_INCLUDED

#include <map>
#include <algorithm>
#include <assert.h>
#include "mp_sdk_common.h"
#include "mp_interface_wrapper.h"

class MpBase;
class MpBase2;
class MpPluginBase;

// Pointer to sound processing member function.
typedef void ( MpBase::* SubProcess_ptr)(int bufferOffset, int sampleFrames);
typedef void ( MpBase2::* SubProcess_ptr2)(int sampleFrames);

#define SET_PROCESS(func)	setSubProcess(static_cast <SubProcess_ptr> (func));
#define SET_PROCESS2(func)	setSubProcess(static_cast <SubProcess_ptr2> (func));

// Handy macros for registering plugin with factory.
#define REGISTER_PLUGIN( className, pluginId ) namespace{ gmpi::IMpUnknown* PASTE_FUNC(create,className)(void){ return static_cast<gmpi::IMpPlugin*> (new className(0)); }; int32_t PASTE_FUNC(r,className) = RegisterPlugin( gmpi::MP_SUB_TYPE_AUDIO, pluginId, &PASTE_FUNC(create,className) );}
#define REGISTER_PLUGIN2( className, pluginId ) namespace{ gmpi::IMpUnknown* PASTE_FUNC(create,className)(void){ return static_cast<gmpi::IMpPlugin2*> (new className()); }; int32_t PASTE_FUNC(r,className) = RegisterPlugin( gmpi::MP_SUB_TYPE_AUDIO, pluginId, &PASTE_FUNC(create,className) );}
// Alias for consistancy with "GMPI_REGISTER_GUI"
#define GMPI_REGISTER_PROCESSOR REGISTER_PLUGIN2

// Pointer to event processing member function.
typedef void (MpPluginBase::* MpBaseMemberPtr)(const gmpi::MpEvent*);


namespace GmpiSdk
{
	class ProcessorHost : public Internal::GmpiIWrapper<gmpi::IMpHost>
	{
	public:
		// Plugin sending out control data.
		inline int32_t setPin(int32_t blockRelativeTimestamp, int32_t pinId, int32_t size, const void* data)
		{
			return Get()->setPin(blockRelativeTimestamp, pinId, size, data);
		}

		// Plugin audio output start/stop (silence detection).
		inline int32_t setPinStreaming(int32_t blockRelativeTimestamp, int32_t pinId, int32_t isStreaming)
		{
			return Get()->setPinStreaming(blockRelativeTimestamp, pinId, isStreaming);
		}

		// Plugin indicates no processing needed until input state changes.
		inline int32_t sleep()
		{
			return Get()->sleep();
		}

		/*
				// Backdoor to GUI. Not recommended. Use Parameters instead to support proper automation.
				virtual int32_t MP_STDCALL sendMessageToGui(int32_t id, int32_t size, const void* messageData) = 0;
		*/
		// Query audio buffer size.
		inline int32_t getBlockSize()
		{
			int32_t s;
			Get()->getBlockSize(s);
			return s;
		}
		/*

		// Query sample-rate.
		virtual int32_t MP_STDCALL getSampleRate(float& returnSampleRate) = 0;

		// Each plugin instance has a host-assigned unique handle shared by UI and Audio class.
		virtual int32_t MP_STDCALL getHandle(int32_t& returnHandle) = 0;

		// Identify Host. e.g. "SynthEdit".
		virtual int32_t MP_STDCALL getHostId(int32_t maxChars, wchar_t* returnString) = 0;

		// Query Host version number. e.g. returnValue of 400000 is Version 4.0
		virtual int32_t MP_STDCALL getHostVersion(int32_t& returnVersion) = 0;

		// Query Host registered user name. "John Smith"
		virtual int32_t MP_STDCALL getRegisteredName(int32_t maxChars, wchar_t* returnName) = 0;

		// Provide list of audio pins.
		virtual int32_t MP_STDCALL createPinIterator(void** returnInterface) = 0;

		// Determin if multiple parallel instances (clones) in use. Clones share same control data and GUI.
		virtual int32_t MP_STDCALL isCloned(int32_t* returnValue) = 0;

		// Provide list of 'clones' - plugins working in parallel to process multiple channels of audio, controled by same parameters/UI.
		virtual int32_t MP_STDCALL createCloneIterator(void** returnInterface) = 0;

		// Provide named shared memory, primarily for sharing waveforms and lookup tables between multiple instances.
		// Use sampleRate of -1 to indicate data is independant of samplerate.
		virtual int32_t MP_STDCALL allocateSharedMemory(const wchar_t* id, void** returnPointer, float sampleRate, int32_t size, int32_t& returnNeedInitialise) = 0;

		// SynthEdit-specific.  Determine file's location depending on host application's conventions. // e.g. "bell.wav" -> "C:/My Documents/bell.wav"
		virtual int32_t MP_STDCALL resolveFilename(const wchar_t* shortFilename, int32_t maxChars, wchar_t* returnFullFilename) = 0;

		// SynthEdit-specific.  Determine file's location depending on host application's conventions. // e.g. "bell.wav" -> "C:/My Documents/bell.wav"
		virtual int32_t MP_STDCALL openProtectedFile(const wchar_t* shortFilename, IProtectedFile **file) = 0;

		inline int32_t setLatency(int32_t latencySamples)
		{
			return Get()->setLatency(latencySamples);
		}
		*/
	};
}

class MpPinBase
{
public:
	MpPinBase() : id_(-1), plugin_(0), eventHandler_(0) {};
	virtual ~MpPinBase(){};
	void initialize( MpPluginBase* plugin, int PinId, MpBaseMemberPtr handler = 0 );

	// overrides for audio pins_
	virtual void setBuffer( float* buffer ) = 0;
	virtual void preProcessEvent( const gmpi::MpEvent* ){};

	virtual void processEvent( const gmpi::MpEvent* e );
	virtual void postProcessEvent( const gmpi::MpEvent* ){};

	int getId(void){return id_;};
	virtual int getDatatype(void) const = 0;
	virtual int getDirection(void) const = 0;
	virtual MpBaseMemberPtr getDefaultEventHandler(void) = 0;
	virtual void sendFirstUpdate() = 0;

protected:
	void sendPinUpdate( int32_t rawSize, void* rawData, int32_t blockPosition = - 1 );
	int id_;
	class MpPluginBase* plugin_;
	MpBaseMemberPtr eventHandler_;
};

template
<typename T, int pinDatatype = MpTypeTraits<T>::PinDataType>
class MpControlPinBase : public MpPinBase
{
public:
	MpControlPinBase() : freshValue_( false )
	{
		// Ensure output pins don't send random garbage at startup.
		setVariableToDefault( value_ );
	}
	MpControlPinBase( T initialValue ) :freshValue_(false), value_( initialValue )
	{
	}
	void sendPinUpdate(int blockPosition = -1)
	{
		MpPinBase::sendPinUpdate( rawSize(), rawData(), blockPosition );
	}
	virtual void setBuffer( float* buffer )
	{
		assert(false && "Control-rate pins_ don't have a buffer");
	}
	const T& getValue(void) const
	{
		assert( id_ != -1 && "remember initializePin() in constructor?" );
		return value_;
	}
	operator T()
	{
		assert( id_ != -1 && "remember initializePin() in constructor?" );
		return value_;
	}
	void setValue( const T &value, int blockPosition = -1 )
	{
		if( !variablesAreEqual<T>(value, value_) ) // avoid unnesc updates
		{
			value_ = value;
			sendPinUpdate( blockPosition );
		}
	}
	const T& operator=(const T &value)
	{
		if( !variablesAreEqual<T>(value, value_) ) // avoid unnesc updates
		{
			value_ = value;
			sendPinUpdate();
		}
		return value_;
	}
	bool operator==(T other)
	{
		assert( plugin_ != 0 && "Don't forget initializePin() on each pin in your constructor." );
		//return other == value_;
		return variablesAreEqual<T>(other, value_);
	}
	virtual void setValueRaw(int size, const void* data)
	{
		//MpTypeTraits<T>::fromRaw(size, data, value_);
		VariableFromRaw<T>(size, data, value_);
	}
	virtual void setValueRaw(size_t size, const void* data)
	{
		VariableFromRaw<T>(static_cast<int>(size), data, value_);
	}
	virtual int rawSize(void) const
	{
		return variableRawSize<T>(value_);
	}
	virtual void* rawData(void)
	{
		return variableRawData<T>(value_);
	}
	virtual int getDatatype(void) const
	{
		return pinDatatype; //MpTypeTraits<T>::PinDataType;
	}
	virtual void preProcessEvent( const gmpi::MpEvent* e )
	{
		switch(e->eventType)
		{
			case gmpi::EVENT_PIN_SET:
				if(e->extraData != 0)
				{
					setValueRaw(e->parm2, e->extraData);
				}
				else
				{
					setValueRaw(e->parm2, &(e->parm3));
				}
				freshValue_ = true;
				break;
		};
	}
	virtual void postProcessEvent( const gmpi::MpEvent* e )
	{
		switch(e->eventType)
		{
			case gmpi::EVENT_PIN_SET:
				freshValue_ = false;
				break;
		};
	}
	virtual MpBaseMemberPtr getDefaultEventHandler(void)
	{
		return 0;
	}
	inline bool isUpdated(void) const
	{
		return freshValue_;
	}
	virtual void sendFirstUpdate()
	{
		sendPinUpdate();
	}

protected:
	T value_;
	bool freshValue_; // true = value_ has been updated by host on current sample_clock
};

template
<typename T, int pinDirection_, int pinDatatype = MpTypeTraits<T>::PinDataType>
class MpControlPin : public MpControlPinBase< T, pinDatatype >
{
public:
	MpControlPin() : MpControlPinBase< T, pinDatatype >()
	{
	}
	MpControlPin( T initialValue ) : MpControlPinBase< T, pinDatatype >( initialValue )
	{
	}
	virtual int getDirection(void) const
	{
		return pinDirection_;
	}
	const T& operator=(const T &value)
	{
		// GCC don't like using plugin_ in this scope. assert( plugin_ != 0 && "Don't forget initializePin() on each pin in your constructor." );
		return MpControlPinBase< T, pinDatatype> ::operator=(value);
	}
	// todo: specialise for value_ vs ref types !!!
	const T& operator=(const MpControlPin<T, gmpi::MP_IN, pinDatatype> &other)
	{
		return operator=(other.getValue());
	}
	const T& operator=(const MpControlPin<T, gmpi::MP_OUT, pinDatatype> &other)
	{
		return operator=(other.getValue());
	}
};

class MpAudioPinBase : public MpPinBase
{
public:
	MpAudioPinBase() :
	    buffer_(nullptr),
		isStreaming_(false)
		{};
	virtual void setBuffer( float* buffer )
	{
		buffer_ = buffer;
	}
	inline float* getBuffer( void ) const
	{
		return buffer_;
	}
	float getValue(int bufferPos = -1) const;
	operator float() const
	{
		return getValue();
	}
	bool operator==(float other)
	{
		return other == getValue();
	}
	virtual void setValueRaw(int size, void*  data)
	{
		assert(false && "Audio-rate pins_ don't support setValueRaw");
	}
	virtual int getDatatype(void) const
	{
		return gmpi::MP_AUDIO;
	}
	virtual MpBaseMemberPtr getDefaultEventHandler(void)
	{
		return 0;
	}
	bool isStreaming(void)
	{
		return isStreaming_;
	}

protected:
	float* buffer_;
	bool isStreaming_; // true = active audio, false = silence or constant input level
};

class AudioInPin : public MpAudioPinBase
{
public:
	AudioInPin() : freshValue_(true){};
	virtual int getDirection(void) const
	{
		return gmpi::MP_IN;
	}
	virtual void preProcessEvent( const gmpi::MpEvent* e );
	virtual void postProcessEvent( const gmpi::MpEvent* e )
	{
		switch(e->eventType)
		{
			case gmpi::EVENT_PIN_STREAMING_START:
			case gmpi::EVENT_PIN_STREAMING_STOP:
				freshValue_ = false;
				break;
		};
	}

	bool isUpdated(void)
	{
		return freshValue_;
	}
	virtual void sendFirstUpdate(){}; // N/A.

private:
	bool freshValue_; // true = value_ has been updated on current sample_clock
};

class AudioOutPin : public MpAudioPinBase
{
public:
	virtual int getDirection(void) const
	{
		return gmpi::MP_OUT;
	}

	// Indicate output pin's value changed, but it's not streaming (a 'one-off' change).
	void setUpdated(int blockPosition = -1)
	{
		setStreaming( false, blockPosition );
	}
	void setStreaming(bool isStreaming, int blockPosition = -1);
	virtual void sendFirstUpdate()
	{
		setStreaming( true );
	}
};

typedef MpControlPin<int, gmpi::MP_IN>			IntInPin;
typedef MpControlPin<int, gmpi::MP_OUT>			IntOutPin;
typedef MpControlPin<float, gmpi::MP_IN>		FloatInPin;
typedef MpControlPin<float, gmpi::MP_OUT>		FloatOutPin;
typedef MpControlPin<MpBlob, gmpi::MP_IN>		BlobInPin;
typedef MpControlPin<MpBlob, gmpi::MP_OUT>		BlobOutPin;
//typedef MpControlPin<std::wstring, gmpi::MP_IN>	StringInPin;
typedef MpControlPin<std::wstring, gmpi::MP_OUT>StringOutPin;

typedef MpControlPin<bool, gmpi::MP_IN>			BoolInPin;
typedef MpControlPin<bool, gmpi::MP_OUT>		BoolOutPin;

// enum (List) pin based on Int Pin
typedef MpControlPin<int, gmpi::MP_IN, gmpi::MP_ENUM>	EnumInPin;
typedef MpControlPin<int, gmpi::MP_OUT, gmpi::MP_ENUM>	EnumOutPin;

//typedef MpAudioPin<gmpi::MP_IN>			AudioInPin;
//typedef MpAudioPin<gmpi::MP_OUT>		AudioOutPin;

class StringInPin : public MpControlPin<std::wstring, gmpi::MP_IN>
{
public:
	explicit operator std::string(); // UTF8 encoded.
};

class MidiInPin : public MpPinBase
{
public:
	virtual int getDatatype(void) const
	{
		return gmpi::MP_MIDI;
	}
	virtual int getDirection(void) const
	{
		return gmpi::MP_IN;
	}

	// These members not needed for MIDI.
	virtual void setBuffer(float* buffer)
	{
		assert( false && "MIDI pins_ don't have a buffer" );
	}
	virtual MpBaseMemberPtr getDefaultEventHandler(void);
	virtual void sendFirstUpdate()
	{
	}
};

class MidiOutPin : public MidiInPin
{
public:
	virtual int getDirection(void) const
	{
		return gmpi::MP_OUT;
	}
	virtual MpBaseMemberPtr getDefaultEventHandler(void)
	{
		assert( false && "output pins don't need event handler" );
		return 0;
	}

	virtual void send(const unsigned char* data, int size, int blockPosition = -1);
};

/*
template<>
MpBaseMemberPtr MpPinMidi<gmpi::MP_IN>::getDefaultEventHandler(void);
template<>
MpBaseMemberPtr MpPinMidi<gmpi::MP_OUT>::getDefaultEventHandler(void);
*/

typedef	std::map<int, MpPinBase*> Pins_t;

class MpPluginBase : public gmpi::IMpPlugin, public gmpi::IMpPlugin2, public IoldSchoolInitialisation
{
	friend class TempBlockPositionSetter;

public:
	MpPluginBase();
	virtual ~MpPluginBase(void){};

	// IMpUnknown methods
	virtual int32_t MP_STDCALL queryInterface(const gmpi::MpGuid& iid, void** returnInterface) override;
	GMPI_REFCOUNT

	virtual int32_t MP_STDCALL open() override;

	// IMpPlugin methods, forwarded to IMpPlugin2 equivalent.
	virtual int32_t MP_STDCALL receiveMessageFromGui( int32_t id, int32_t size, void* messageData ) override
	{
		int32_t r = gmpi::MP_OK;
		if( !recursionFix )
		{
			recursionFix = true;
			// When hosted as IMpPlugin, forward call to IMpPlugin2 version.
			r = receiveMessageFromGui( id, size, (const void*) messageData );
			recursionFix = false;
		}
		return r;
	}

	virtual void MP_STDCALL process( int32_t count, gmpi::MpEvent* events ) override
	{
		// When hosted as IMpPlugin, forward call to IMpPlugin2 version.
		process(count, ( const gmpi::MpEvent* ) events);
	}

	// IMpPlugin2 methods
	virtual int32_t MP_STDCALL setHost(IMpUnknown* host) override;
	virtual int32_t MP_STDCALL setBuffer(int32_t pinId, float* buffer) override;
	virtual void MP_STDCALL process( int32_t count, const gmpi::MpEvent* events ) override = 0;
	virtual int32_t MP_STDCALL receiveMessageFromGui( int32_t id, int32_t size, const void* messageData ) override
	{
		int32_t r = gmpi::MP_OK;
		if( !recursionFix )
		{
			recursionFix = true;
			// When hosted as IMpPlugin, forward call to IMpPlugin2 version.
			r = receiveMessageFromGui(id, size, (void*)messageData);
			recursionFix = false;
		}
		return r;
	}
	// Methods
	void sendEvent(){};
	void initializePin( int PinId, MpPinBase& pin, MpBaseMemberPtr handler = 0 );
	void initializePin( MpPinBase &pin, MpBaseMemberPtr handler = 0 )
	{
		int idx;
		if (!pins_.empty())
		{
			idx = pins_.rbegin()->first + 1;
		}
		else
		{
			idx = 0;
		}
		initializePin(idx, pin, handler); // Automatic indexing.
	}

	inline int getBlockPosition(void)
	{
		return blockPos_;
	}
	void preProcessEvent( const gmpi::MpEvent* e );
	void processEvent( const gmpi::MpEvent* e );
	void postProcessEvent( const gmpi::MpEvent* e );
	virtual void onMidiMessage(int pin, const unsigned char* midiMessage, int size) // size < 4 for short msg, or > 4 for sysex.
    {
        onMidiMessage( pin, ( unsigned char*) midiMessage, size ); // cope seamlessly with older plugins.
    }
    virtual void onMidiMessage( int pin, unsigned char* midiMessage, int size ){}; // deprecated, non-const correct version of above.
	virtual void onSetPins(void){}  // one or more pins_ updated.  Check pin update flags to determine which ones.
	virtual void OnPinStreamingChange( bool isStreaming ) = 0;
	inline gmpi::IMpHost* getHost( void )
	{
		assert(host.Get() != nullptr); // Please don't call host from contructor. Use initialize() instead.
		return host.Get();
	}

	void midiHelper( const gmpi::MpEvent* e );
	int getBlockSize(void)
	{
		int returnValue;
		getHost()->getBlockSize(returnValue);
		return returnValue;
	}
	float getSampleRate(void)
	{
		float returnValue;
		getHost()->getSampleRate(returnValue);
		return returnValue;
	}
	void resetSleepCounter();
	void nudgeSleepCounter()
	{
		sleepCount_ = (std::max)(sleepCount_, 1);
	}
	virtual void onGraphStart();	// called on very first sample.

protected:
//	gmpi_sdk::mp_shared_ptr<gmpi::IMpHost> host_;
	GmpiSdk::ProcessorHost host;
	Pins_t pins_;

	int blockPos_;				// valid only during processEvent()
	int sleepCount_;			// sleep countdown timer.
	int streamingPinCount_;		// tracks how many pins streaming.
	enum { SLEEP_AUTO = -1, SLEEP_DISABLE, SLEEP_ENABLE } canSleepManualOverride_;
	bool eventsComplete_;		// Flag indicates all events have been delt with, and module is safe to sleep.
	bool recursionFix;

public:
#if defined(_DEBUG)
	bool debugIsOpen_;
	bool blockPosExact_;
	bool debugGraphStartCalled_;
#endif
};

// When sending values out a pin, it's cleaner if the block-position is known,
// however in subProcess(), we can't usually identify a specific block position automatically.
// This helper class lets you set the block-position manually in able to use shorthand pin setting syntax. e.g. pinOut = 3;
class TempBlockPositionSetter
{
	bool saveBlockPosExact_;
	int saveBlockPosition_;
	MpPluginBase* module_;

public:
	TempBlockPositionSetter(MpPluginBase* module, int currentBlockPosition)
	{
		module_ = module;
		saveBlockPosition_ = module_->getBlockPosition();
		module_->blockPos_ = currentBlockPosition;

#if defined(_DEBUG)
		saveBlockPosExact_ = module->blockPosExact_;
		module_->blockPosExact_ = true;
#endif
	}

	~TempBlockPositionSetter()
	{
		module_->blockPos_ = saveBlockPosition_;
#if defined(_DEBUG)
		module_->blockPosExact_ = saveBlockPosExact_;
#endif
	}
};

class MpBase: public MpPluginBase
{
public:
	MpBase(gmpi::IMpUnknown *unused) : MpPluginBase()
	,curSubProcess_( &MpBase::subProcessPreSleep )
	,saveSubProcess_( &MpBase::subProcessNothing )
	{
	}

	virtual void MP_STDCALL process(int32_t count, const gmpi::MpEvent* events);
	void setSubProcess(SubProcess_ptr func);
	inline int blockPosition(void) // wrong coding standard, retained for backward compatibility. Use getBlockPosition() instead.
	{
		return blockPos_;
	}
	SubProcess_ptr getSubProcess(void)
	{
		return saveSubProcess_;
	}
	void setSleep(bool isOkToSleep);
	void OnPinStreamingChange( bool isStreaming );

protected:
	// the default routine, if none set by module.
	void subProcessNothing(int bufferOffset, int sampleFrames){};
	void subProcessPreSleep(int bufferOffset, int sampleFrames);

private:
	SubProcess_ptr curSubProcess_;
	SubProcess_ptr saveSubProcess_; // saves curSubProcess_ while sleeping
};

// Experimental advanced plugin base-class with simplified sub_process method.
/*
 To upgrade:
 * Replace references to MpBase with MpBase2
 * change REGISTER_PLUGIN to REGISTER_PLUGIN2
 * remove "IMpUnknown* host" from constructors.
 * Remove "int bufferOffset," parameter from processing methods.
 * change assignments in processing -
		float* signal = bufferOffset + pinSignal.getBuffer();
	to
		float* signal = getBuffer( pinSignal );

 * Change any "SET_PROCESS" to "SET_PROCESS2"
 * Change any "SubProcess_ptr" to "SubProcess_ptr2"

 getBlockPosition() supplies buffer offset if needed.
 */

class MpBase2: public MpPluginBase
{
public:
	MpBase2() : MpPluginBase()
	,curSubProcess_( &MpBase2::subProcessPreSleep )
	,saveSubProcess_( &MpBase2::subProcessNothing )
	{
	}

	virtual void MP_STDCALL process(int32_t count, const gmpi::MpEvent* events);
	// TODO: Should return const float* for input pins to prevent inadvertant modification of input buffer.
	inline float* getBuffer( const MpAudioPinBase& pin ) const
	{
		return blockPos_ + pin.getBuffer();
	}

	template <typename PointerToMemeber>
	void setSubProcess(PointerToMemeber functionPointer)
	{
		// under normal use, curSubProcess_ and saveSubProcess_ are the same.
		if (curSubProcess_ == saveSubProcess_)
		{
			curSubProcess_ = saveSubProcess_ = static_cast <SubProcess_ptr2> (functionPointer);
		}
		else // in sleep mode.
		{
			saveSubProcess_ = static_cast <SubProcess_ptr2> (functionPointer);
			assert(saveSubProcess_ != &MpBase2::subProcessPreSleep);
		}
	}

	inline SubProcess_ptr2 getSubProcess(void)
	{
		return saveSubProcess_;
	}
	void setSleep(bool isOkToSleep);
	void OnPinStreamingChange( bool isStreaming );

	// the default routine, if none set by module.
	void subProcessNothing(int sampleFrames){};
	void subProcessPreSleep(int sampleFrames);

	SubProcess_ptr2 curSubProcess_;

private:
	SubProcess_ptr2 saveSubProcess_; // saves curSubProcess_ while sleeping
};

#endif // .H INCLUDED
