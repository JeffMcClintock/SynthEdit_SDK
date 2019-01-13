#pragma once
#include <map>
#include <memory>
#include "../../../SynthEdit/IGuiHost.h"
#include "../../../se_vst3/source/SeParameter_vst3.h"
#include "interThreadQue.h"
#include "TimerManager.h"
#include "../se_vst3/source/StagingMemoryBuffer.h"
#include "SeAudioMaster.h"
#include "mp_gui.h"

/*
#include "Controller.h"
*/

// Manages plugin parameters.

class MpController : public IGuiHost2, public interThreadQueUser, public TimerClient
{
protected:
	std::vector< std::unique_ptr<SeParameter_vst3> > parameters_;
	std::map<int, SeParameter_vst3_native* > vst3TagToParameter;	// DAW parameter Index to parameter
	std::map< std::pair<int, int>, int > moduleParameterIndex;		// Module Handle/ParamID to Param Handle.
	std::map< int, SeParameter_vst3* > ParameterHandleIndex;		// Param Handle to Parameter*.
	std::vector<gmpi::IMpParameterObserver*> m_guis2;
	GmpiGui::FileDialog nativeFileDialog;

#ifdef SE_TARGET_VST3
    // Hold data until timer can put it in VST3 queue mechanism.
	StagingMemoryBuffer queueToDsp_;
#else
    interThreadQue queueToDsp_;
#endif
    interThreadQue message_que_dsp_to_ui;
	bool hasInternalPresets;

    std::vector<SeParameter_vst3_native* > vst3Parameters; // flat list.
	std::vector<std::string> internalProgramNames;

	void OnFileDialogComplete(int mode, int32_t result);

public:
	MpController() :
		message_que_dsp_to_ui(UI_MESSAGE_QUE_SIZE2)
        ,queueToDsp_(AUDIO_MESSAGE_QUE_SIZE)
		, hasInternalPresets(false)
	{}

	int nativeGetParameterCount()
	{
		return static_cast<int>(vst3Parameters.size());
	}

	SeParameter_vst3* nativeGetParameterByIndex(int nativeIndex)
	{
		if (nativeIndex >= 0 && nativeIndex < static_cast<int>(vst3Parameters.size()))
			return vst3Parameters[nativeIndex];

		return nullptr;
	}

	SeParameter_vst3_native* nativeGetParameter(int nativeId)
	{
		auto it = vst3TagToParameter.find(nativeId);
		if (it != vst3TagToParameter.end())
		{
			return (*it).second;
		}

		return nullptr;
	}

	void AddNativeParameter(int nativeTag, SeParameter_vst3_native* p);

	// Override these
	virtual bool sendMessageToProcessor(const void* data, int size) = 0;
	void ParamToDsp(SeParameter_vst3* param, int32_t voice = 0);
	virtual void ParamGrabbed(SeParameter_vst3_native* param, int32_t voice = 0) = 0;
	virtual void ParamToProcessorViaHost(SeParameter_vst3_native* param, int32_t voice = 0) = 0;

	void Initialize(class TiXmlElement* controllerE);
	virtual int32_t sendSdkMessageToAudio(int32_t handle, int32_t id, int32_t size, const void* messageData) override;
	void OnSetHostControl(int hostControl, int32_t paramField, int32_t size, const void * data, int32_t voice);

	// IGuiHost2
	virtual int32_t RegisterGui2(gmpi::IMpParameterObserver* gui) override
	{
		m_guis2.push_back(gui);
		return gmpi::MP_OK;
	}
	virtual int32_t UnRegisterGui2(gmpi::IMpParameterObserver* gui) override
	{
		for (auto it = m_guis2.begin(); it != m_guis2.end(); ++it)
		{
			if (*it == gui)
			{
				m_guis2.erase(it);
				break;
			}
		}

		return gmpi::MP_OK;
	}
	virtual void initializeGui(gmpi::IMpParameterObserver* gui, int32_t parameterHandle, ParameterFieldType FieldId, int32_t voice) override;
	virtual int32_t getParameterHandle(int32_t moduleHandle, int32_t moduleParameterId) override;
	virtual int32_t getParameterModuleAndParamId(int32_t parameterHandle, int32_t* returnModuleHandle, int32_t* returnModuleParameterId) override;
	virtual RawView getParameterValue(int32_t parameterHandle, int32_t moduleFieldId, int32_t voice = 0) override;

	// Presets
	virtual std::string loadNativePreset(std::wstring sourceFilename) = 0;
	virtual void saveNativePreset(const char * filename, const std::string& xml) = 0;
	virtual std::wstring getNativePresetExtension() = 0;

	void LoadInternalPreset(int preset);
	void LoadNativePresetFile(std::string presetName) override;
	void ImportPresetXml(const char* filename, int presetIndex = -1);
	std::string getPresetXml();
	void setPreset(class TiXmlNode* parentXml, bool updateProcessor, int preset);
	void setPreset(const std::string& xml, bool updateProcessor = true, int preset = 0);
	void ExportPresetXml(const char* filename);
	void ImportBankXml(const char * filename);
	void ExportBankXml(const char * filename);

	virtual void setParameterValue(RawView value, int32_t parameterHandle, int32_t moduleFieldId = FT_VALUE, int32_t voice = 0) override;
	virtual int32_t resolveFilename(const wchar_t* shortFilename, int32_t maxChars, wchar_t* returnFullFilename) override;

	void setParameterNormalisedFromHost(int32_t hostParamterId, double normalized);

	void updateGuis(SeParameter_vst3* parameter, int voice)
	{
		auto rawValue = parameter->getValueRaw(FT_VALUE, voice);
		float normalized = parameter->getNormalized(); // voice !!!?

		for (auto pa : m_guis2)
		{
			// Update value.
			pa->setParameter(parameter->parameterHandle_, FT_VALUE, voice, rawValue.data(), (int32_t)rawValue.size());

			// Update normalized.
			pa->setParameter(parameter->parameterHandle_, FT_NORMALIZED, voice, &normalized, (int32_t)sizeof(normalized));
		}
	}

	void updateGuis(SeParameter_vst3* parameter, ParameterFieldType fieldType, int voice = 0 )
	{
		auto rawValue = parameter->getValueRaw(fieldType, voice);

		for (auto pa : m_guis2)
		{
			pa->setParameter(parameter->parameterHandle_, fieldType, voice, rawValue.data(), (int32_t)rawValue.size());
		}
	}

	// interThreadQueUser
	virtual bool OnTimer() override;
	virtual bool onQueMessageReady(int handle, int msg_id, class my_input_stream& p_stream) override;

	virtual void serviceGuiQueue() override
	{
		message_que_dsp_to_ui.pollMessage(this);
	}

	virtual void ResetProcessor() {}
	virtual void OnStartPresetChange() {}
	virtual void OnEndPresetChange() {}
};
