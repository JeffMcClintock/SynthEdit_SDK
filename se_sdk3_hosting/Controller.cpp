#pragma once
#include "Controller.h"
#include "tinyxml/tinyxml.h"
#include "../se_sdk2/se_datatypes.h"
#include "RawConversions.h"
#include <codecvt>
#include <locale>
#include "midi_defs.h"
#include "HostControls.h"
#include "GuiPatchAutomator3.h"
#include "BundleInfo.h"
#include "modules/shared/FileFinder.h"
#include "../tinyXml2/tinyxml2.h"

#ifndef SE_TARGET_VST3
#include "my_msg_que_output_stream.h"
#endif

using namespace std;

void MpController::Initialize(class TiXmlElement* controllerE)
{
	auto patchManagerE = controllerE->FirstChildElement();
	assert(strcmp(patchManagerE->Value(), "PatchManager") == 0);

	patchManagerE->QueryBoolAttribute("HasInternalPresets", &hasInternalPresets);

	// Preset Names
	if (hasInternalPresets)
	{
		auto presetnamesE = patchManagerE->FirstChildElement("PresetNames");
		for (auto nameE = presetnamesE->FirstChildElement("Preset"); nameE; nameE = nameE->NextSiblingElement("Preset"))
		{
			internalProgramNames.push_back(nameE->Attribute("Name"));
		}
	}

	std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;

	auto parameters_xml = patchManagerE->FirstChildElement("Parameters");

	for (auto parameter_xml = parameters_xml->FirstChildElement("Parameter"); parameter_xml; parameter_xml = parameter_xml->NextSiblingElement("Parameter"))
	{
		int dataType = DT_FLOAT;
		int ParameterIndex = -1;
		int ParameterHandle = -1;
		int Private = 0;

		std::string Name = parameter_xml->Attribute("Name");
		parameter_xml->QueryIntAttribute("ValueType", &dataType);
		parameter_xml->QueryIntAttribute("Index", &ParameterIndex);
		parameter_xml->QueryIntAttribute("Handle", &ParameterHandle);
		parameter_xml->QueryIntAttribute("Private", &Private);

		if (dataType == DT_TEXT || dataType == DT_BLOB)
		{
			Private = 1; // VST and AU can't handle this type of parameter.
		}
		else
		{
			if (Private != 0)
			{
				// Check parameter is numeric and a valid type.
				assert(dataType == DT_ENUM || dataType == DT_DOUBLE || dataType == DT_BOOL || dataType == DT_FLOAT || dataType == DT_INT || dataType == DT_INT64);
			}
		}

		int stateful_ = 1;
		parameter_xml->QueryIntAttribute("persistant", &stateful_);
		int hostControl = -1;
		parameter_xml->QueryIntAttribute("HostControl", &hostControl);
		int ignorePc = 0;
		parameter_xml->QueryIntAttribute("ignoreProgramChange", &ignorePc);

		//		const wchar_t* units = nullptr;
		double pminimum = 0.0;
		double pmaximum = 10.0;

		parameter_xml->QueryDoubleAttribute("RangeMinimum", &pminimum);
		parameter_xml->QueryDoubleAttribute("RangeMaximum", &pmaximum);

		int moduleHandle_ = -1;
		int moduleParamId_ = 0;
		bool isPolyphonic_ = false;
		wstring enumList_;

		parameter_xml->QueryIntAttribute("Module", &(moduleHandle_));
		parameter_xml->QueryIntAttribute("ModuleParamId", &(moduleParamId_));
		parameter_xml->QueryBoolAttribute("isPolyphonic", &(isPolyphonic_));

		if (dataType == DT_INT || dataType == DT_TEXT /*|| dataType == DT_ENUM */)
		{
			auto s = parameter_xml->Attribute("MetaData");
			if (s)
				enumList_ = convert.from_bytes(s);
		}

		SeParameter_vst3_private* seParameter = nullptr;

		if (Private == 0)
		{
			auto param = new SeParameter_vst3_native(this);

			vst3TagToParameter.insert(make_pair(ParameterIndex, param));
			vst3Parameters.push_back(param);

			param->hostIndex_ = ParameterIndex;
			seParameter = param;
		}
		else
		{
			auto param = new SeParameter_vst3_private(this);
			seParameter = param;
			param->isPolyphonic_ = isPolyphonic_;
		}

		seParameter->hostControl_ = hostControl;
		seParameter->minimum = pminimum;
		seParameter->maximum = pmaximum;

		// Preset values from patch list.
		auto patch_xml = parameter_xml->FirstChildElement("patch-list");
		if (patch_xml)
		{
//?			int voiceCount = isPolyphonic_ ? 128 : 1;
			int presetCount = ignorePc ? 1 : 128;

			int voice = 0;
			int preset = 0;
			for (; patch_xml; patch_xml = patch_xml->NextSiblingElement()) // for each voice.
			{
				std::vector<std::string> presetValues;
				XmlSplitString(patch_xml->GetText(), presetValues);
				assert(presetValues.size() == presetCount);

				int p = (std::min)(preset, (int)presetValues.size() - 1);
				seParameter->rawValues_.push_back(ParseToRaw(dataType, presetValues[p]));
				++voice;
			}
		}

		seParameter->parameterHandle_ = ParameterHandle;
		seParameter->datatype_ = dataType;
		seParameter->moduleHandle_ = moduleHandle_;
		seParameter->moduleParamId_ = moduleParamId_;
		seParameter->stateful_ = stateful_;
		seParameter->name_ = convert.from_bytes(Name);
		seParameter->enumList_ = enumList_;
//		seParameter->ignoreProgramChange = ignorePc != 0;

		parameters_.push_back(std::unique_ptr<SeParameter_vst3>(seParameter));
		ParameterHandleIndex.insert(std::make_pair(ParameterHandle, seParameter));
		moduleParameterIndex.insert(std::make_pair(std::make_pair(moduleHandle_, moduleParamId_), ParameterHandle));
        
        // Ensure host queries return correct value.
        seParameter->upDateImmediateValue();
	}
}

// This is for "fake" parameters representing MIDI controllers.
void MpController::AddNativeParameter(int nativeTag, SeParameter_vst3_native* p)
{
	vst3TagToParameter.insert(make_pair(nativeTag, p));
	vst3Parameters.push_back(p);

	parameters_.push_back(std::unique_ptr<SeParameter_vst3>(p));

	p->parameterHandle_ = -1;

//?	p->stateful_ = false;
	/* seems only to clash with valid handles, not needed unless parameter accessed by GUI system. If so genereate a negative handle or somit.
	// try to generate unique handle (all regular params need to be added first).
	auto ParameterHandle = 0;
	if(!ParameterHandleIndex.empty())
		ParameterHandle = ParameterHandleIndex.rbegin()->first + 1;

	ParameterHandleIndex.insert(std::make_pair(ParameterHandle, p));
	*/
}

void MpController::setParameterNormalisedFromHost(int32_t hostParamterId, double normalized)
{
	auto p = nativeGetParameter(hostParamterId);
	if (p)
	{
		float n = static_cast<float>(normalized);
        p->SeParameter_vst3_private::setParameterRaw(FT_NORMALIZED, sizeof(n), &n);
	}
}

void MpController::setParameterValue(RawView value, int32_t parameterHandle, int32_t paramField, int32_t voice)
{
	auto it = ParameterHandleIndex.find(parameterHandle);
	if (it != ParameterHandleIndex.end())
	{
		// Special case for MIDI Learn
		if (paramField == FT_MENU_SELECTION)
		{
			auto choice = RawToValue<int32_t>(value.data(), value.size());

			// 0 not used as it gets passed erroneously during init
			int cc = 0;
			if (choice == 1) // learn
			{
				cc = ControllerType::Learn;
				//				SetValueRaw(FT_AUTOMATION, &temp, sizeof(temp));
			}
			else
			{
				if (choice == 2) // un-learn
				{
					cc = ControllerType::None;
					//					SetValueRaw(FT_AUTOMATION, &temp, sizeof(temp));
				}
			}
			/*
			if( choice == 3 ) // Set via dialog
			{
			dlg_assign_controller dlg(getPatchManager(), this, CWnd::GetDesktopWindow());
			dlg.DoModal();
			}
			*/
			// Send MIDI learn message to DSP.
			//---send a binary message
			if (cc != 0)
			{
                /*
				StagingMemoryBufferOutputStream s(&queueToDsp_);
				s << (int)parameterHandle;
				s << 'D' << 'I' << 'C' << 'C';  // "CCID"
*/
#ifdef SE_TARGET_VST3
                StagingMemoryBufferOutputStream s(&queueToDsp_);
                
                s << (int)parameterHandle;
                s << 'D' << 'I' << 'C' << 'C';  // "CCID"
#else
                my_msg_que_output_stream s(&queueToDsp_, (int32_t)parameterHandle, "CCID");
#endif
                
				s << (int) sizeof(int);
				s << cc;
                s.Send();
			}
		}
		else
		{
			auto seParameter = (*it).second;
			if (seParameter->setParameterRaw(paramField, value.size(), value.data(), voice))
			{
				seParameter->updateProcessor(paramField, voice);
			}
		}
	}
}

void MpController::OnSetHostControl(int hostControl, int32_t paramField, int32_t size, const void* data, int32_t voice)
{
	switch (hostControl)
	{
	case HC_PROGRAM:
		if (paramField == FT_VALUE)
		{
			auto preset = RawToValue<int32_t>(data, size);
/*todo
			auto p = getParameterObject(ProgramChangeParameterId);
			p->setNormalized(p->toNormalized(patch));
			performEdit(ProgramChangeParameterId, p->getNormalized()); // this seems to send the value to Host/DSP.
*/
			SeParameter_vst3* programNameParam = nullptr;
			SeParameter_vst3* programNamesParam = nullptr;
			for (auto& p : parameters_)
			{
				if (p->getHostControl() == HC_PROGRAM_NAME)
				{
					programNameParam = p.get();
				}
				else
				{
					if (p->getHostControl() == HC_PROGRAM_NAMES_LIST)
					{
						programNamesParam = p.get();
					}
				}
			}

			if (programNamesParam == nullptr || programNameParam == nullptr)
				return;
			auto raw = programNamesParam->getValueRaw(FT_VALUE, 0);
			it_enum_list it(RawToValue<wstring>(raw.data(), raw.size()));
/*
			if (preset >= 200) // Presets scanned off Cubase preset folder.
			{
				it.FindValue(preset);
				if (!it.IsDone())
				{
					auto raw2 = ToRaw4(it.CurrentItem()->text);
					programNameParam->setParameterRaw(FT_VALUE, raw2.size(), raw2.data());

					LoadNativePresetFile( WStringToUtf8( it.CurrentItem()->text ) );
				}
			}
			else
*/
			{
				if (hasInternalPresets)
				{
					it.FindIndex(preset);
					if (!it.IsDone())
					{
						auto raw2 = ToRaw4(it.CurrentItem()->text);
						programNameParam->setParameterRaw(FT_VALUE, raw2.size(), raw2.data());
					}

					LoadInternalPreset(preset);
				}
			}
		}
		break;

	case HC_PATCH_COMMANDS:
		if (paramField == FT_VALUE)
		{
			int patchCommand = *(int32_t*)data; // load=2 or save=3.

			if (patchCommand > 0)
			{
				for (auto g : m_guis2)
				{
					auto pa = dynamic_cast<GuiPatchAutomator3*>(g);
					if (pa)
					{
						auto gh = dynamic_cast<gmpi_gui::IMpGraphicsHost*>(pa->getHost());
						if (gh)
						{
//!!!! TODO on MAc
#if 1 //def _WIN32
							// L"Load Preset=2,Save Preset,Import Bank,Export Bank"
							int dialogMode = (patchCommand == 2 || patchCommand == 4) ? 0 : 1; // load or save.
							gh->createFileDialog(dialogMode, nativeFileDialog.GetAddressOf());

							if (!nativeFileDialog.isNull())
							{
								if (patchCommand > 3)
								{
									nativeFileDialog.AddExtension("xmlbank", "XML Bank");
									auto fullPath = WStringToUtf8(BundleInfo::instance()->getUserDocumentFolder());
									combinePathAndFile(fullPath.c_str(), "bank.xmlbank");
									nativeFileDialog.SetInitialFullPath(fullPath);
								}
								else
								{
#ifdef _WIN32
									nativeFileDialog.AddExtension("vstpreset", "VST3 Preset");
#else
                                    nativeFileDialog.AddExtension("aupreset", "Audio Unit Preset");
#endif
									nativeFileDialog.AddExtension("xmlpreset", "XML Preset");
									nativeFileDialog.AddExtension("*", "All Files");
									nativeFileDialog.SetInitialFullPath( WStringToUtf8(BundleInfo::instance()->getPresetFolder()) );
								}

								nativeFileDialog.ShowAsync([this, patchCommand](int32_t result) -> void { this->OnFileDialogComplete(patchCommand, result); });
							}
#endif
							break;
						}
					}
				}
			}
		}

		break;
	}
}

int32_t MpController::sendSdkMessageToAudio(int32_t handle, int32_t id, int32_t size, const void* messageData)
{
    /*
	StagingMemoryBufferOutputStream s(&queueToDsp_);
	s << handle;
	s << '\0' << 'k' << 'd' << 's';  // "sdk"
    */
#ifdef SE_TARGET_VST3
    StagingMemoryBufferOutputStream s(&queueToDsp_);
    
    s << (int)handle;
    s << '\0' << 'k' << 'd' << 's';  // "sdk"
#else
    my_msg_que_output_stream s(&queueToDsp_, (int32_t)handle, "sdk");
#endif
    
	s << (int32_t)(size + 2 * sizeof(int32_t)); // size of ID plus sizeof message.

	s << id;

	s << size;
	s.Write(messageData, size);

    s.Send();
    
	return gmpi::MP_OK;
}


void MpController::ParamToDsp(SeParameter_vst3* param, int32_t voice)
{
	assert(dynamic_cast<SeParameter_vst3_hostControl*>(param) == nullptr); // These have (not) "unique" handles that may map to totally random DSP parameters.

	//---send a binary message
	{
		bool isVariableSize = param->datatype_ == DT_TEXT || param->datatype_ == DT_BLOB;

		auto raw = param->getValueRaw(FT_VALUE, voice);

		bool due_to_program_change = false;
		int32_t recievingMessageLength = (int)(sizeof(int32_t) + sizeof(bool) + raw.size());
		if (isVariableSize)
		{
			recievingMessageLength += (int)sizeof(int32_t);
		}

		if (param->isPolyPhonic())
		{
			recievingMessageLength += (int)sizeof(int32_t);
		}

		int patch = 0;

#ifdef SE_TARGET_VST3
        StagingMemoryBufferOutputStream s(&queueToDsp_);
        
        s << (int)param->parameterHandle_;
        s << '\0' << 'c' << 'p' << 'p';  // "ppc"
#else
        my_msg_que_output_stream s(&queueToDsp_, (int32_t)param->parameterHandle_, "ppc");
#endif

		s << recievingMessageLength;
		s << due_to_program_change;
		s << patch;

		if (param->isPolyPhonic())
		{
			s << voice;
		}

		if (isVariableSize)
		{
			s << (int32_t)raw.size();
		}

		s.Write(raw.data(), (unsigned int)raw.size());
        
        s.Send();
	}
}

int32_t MpController::getParameterHandle(int32_t moduleHandle, int32_t moduleParameterId)
{
	int hostControl = -1 - moduleParameterId;

	if (hostControl >= 0)
	{
		// why not just shove it in with negative handle? !!! A: becuase of potential attachment to container.
		for (auto& p : parameters_)
		{
			if (p->getHostControl() == hostControl && (moduleHandle == -1 || moduleHandle == p->ModuleHandle()))
			{
				return p->parameterHandle_;
				break;
			}
		}

		SeParameter_vst3_hostControl* p = nullptr;

		switch (hostControl)
		{
		case HC_PATCH_COMMANDS:
			p = new SeParameter_vst3_hostControl(this, hostControl);
			//p->enumList_=L"Copy Patch=1,Load Preset,Save Preset,Load Bank,Save Bank";
			p->enumList_ = L"Load Preset=2,Save Preset,Import Bank,Export Bank";
			break;

		case HC_PROGRAM:
			p = new SeParameter_vst3_hostControl(this, hostControl);
			break;

		case HC_PROGRAM_NAME:
			p = new SeParameter_vst3_hostControl(this, hostControl);
			{
				auto raw2 = ToRaw4(L"Factory");
				p->setParameterRaw(FT_VALUE, (int32_t)raw2.size(), raw2.data());
			}
			break;

		case HC_PROGRAM_NAMES_LIST:
		{
			auto param = new SeParameter_vst3_hostControl(this, hostControl);
			p = param;
			p->datatype_ = DT_TEXT;

			if (hasInternalPresets) // Always true at present.
			{
				std::wostringstream oss;
				bool first = true;
				std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
				for (auto& pn : internalProgramNames)
				{
					if (first)
					{
						first = false;
					}
					else
					{
						oss << L',';
					}
					oss << convert.from_bytes(pn);
				}
				param->rawValues_.push_back(ToRaw4(oss.str()));
			}
			else
			{
				param->rawValues_.push_back(ToRaw4(L"No Internal Presets"));
			}
		}
		break;
		/* what would it do?
		case HC_MIDI_CHANNEL:
		break;
		*/
		}

		if (p)
		{
			p->stateful_ = false;

			// clashes with valid handles on DSP, ensure NEVER sent to DSP!!

			// generate unique parameter handle, assume all other parameters already registered.
			p->parameterHandle_ = 0;
			if (!ParameterHandleIndex.empty())
				p->parameterHandle_ = ParameterHandleIndex.rbegin()->first + 1;

			ParameterHandleIndex.insert(std::make_pair(p->parameterHandle_, p));
			parameters_.push_back(std::unique_ptr<SeParameter_vst3>(p));

			return p->parameterHandle_;
		}
	}
	else
	{
		auto it = moduleParameterIndex.find(std::make_pair(moduleHandle, moduleParameterId));
		if (it != moduleParameterIndex.end())
			return (*it).second;
	}

	return -1;
}

void MpController::initializeGui(gmpi::IMpParameterObserver* gui, int32_t parameterHandle, ParameterFieldType FieldId, int32_t voice)
{
	auto it = ParameterHandleIndex.find(parameterHandle);

	if (it != ParameterHandleIndex.end())
	{
		auto p = (*it).second;

		auto raw = p->getValueRaw(FieldId, voice);
		gui->setParameter(parameterHandle, FieldId, voice, raw.data(), (int32_t)raw.size());
	}
}

bool MpController::onQueMessageReady(int recievingHandle, int recievingMessageId, class my_input_stream& p_stream)
{
	auto it = ParameterHandleIndex.find(recievingHandle);
	if (it != ParameterHandleIndex.end())
	{
		auto p = (*it).second;
		p->updateFromDsp(recievingMessageId, p_stream);
		return true;
	}
	else
	{
		if (recievingMessageId == id_to_long("sdk"))
		{
			int32_t id;
			int32_t size;
			void* messageData;

			p_stream >> id;
			p_stream >> size;
			messageData = malloc(size);
			p_stream.Read(messageData, size);

			free(messageData);
			return true;
		}
		else
		{
			if (recievingMessageId == id_to_long("rest")) // Reset Processor (e.g. latency changed).
			{
				ResetProcessor();
			}
		}
	}
	return false;
}

bool MpController::OnTimer()
{
	message_que_dsp_to_ui.pollMessage(this);

#ifdef SE_TARGET_VST3
    if (!queueToDsp_.empty())
	{
		sendMessageToProcessor(queueToDsp_.data(), queueToDsp_.size());
		queueToDsp_.clear();
/*
		auto message = allocateMessage();
		if (message)
		{
			FReleaser msgReleaser(message);
			message->setMessageID("BinaryMessage");

			message->getAttributes()->setBinary("MyData", queueToDsp_.data(), queueToDsp_.size());
			sendMessage(message);

			queueToDsp_.clear();
		}
*/
	}
#endif

	return true;
}

int32_t MpController::resolveFilename(const wchar_t* shortFilename, int32_t maxChars, wchar_t* returnFullFilename)
{
	// copied from CSynthEditAppBase.

	std::wstring l_filename(shortFilename);
	std::wstring file_ext;
	file_ext = GetExtension(l_filename);

	// need to add path?
#ifdef _WIN32
    bool has_root_path = l_filename.find(':') != string::npos;
#else
    bool has_root_path = l_filename.size() > 0 && l_filename[0] == L'/';
#endif
    
	if ( !has_root_path)
	{
		auto default_path = BundleInfo::instance()->getImbeddedFileFolder();

		l_filename = combine_path_and_file(default_path, l_filename);
	}

	if (l_filename.size() >= static_cast<size_t> (maxChars))
	{
		// return empty string (if room).
		if (maxChars > 0)
			returnFullFilename[0] = 0;

		return gmpi::MP_FAIL;
	}

	WStringToWchars(l_filename, returnFullFilename, maxChars);

	return gmpi::MP_OK;
}

void MpController::OnFileDialogComplete(int patchCommand, int32_t result)
{
	if (result == gmpi::MP_OK)
	{
		auto fullpath = nativeFileDialog.GetSelectedFilename();
		auto filetype = GetExtension(fullpath);
		bool isXmlPreset = filetype == "xmlpreset";

		switch (patchCommand) // L"Load Preset=2,Save Preset,Import Bank,Export Bank"
		{
		case 2:
			if (isXmlPreset)
				ImportPresetXml(fullpath.c_str());
			else
			{
				auto xml = loadNativePreset( Utf8ToWstring(fullpath) );
				setPreset(xml);
			}
			break;

		case 3:
			if (isXmlPreset)
				ExportPresetXml(fullpath.c_str());
			else
				saveNativePreset(fullpath.c_str(), getPresetXml());
			break;

		case 4:
			ImportBankXml(fullpath.c_str());
			break;

		case 5:
			ExportBankXml(fullpath.c_str());
			break;
		}
	}

	nativeFileDialog.setNull(); // release it.
}

void MpController::ImportPresetXml(const char* filename, int presetIndex)
{
	int preset = (std::max)(0, presetIndex);

	TiXmlDocument doc;
	doc.LoadFile(filename);

	if (doc.Error())
	{
		assert(false);
		return;
	}

	bool isPreset = true;

	TiXmlHandle hDoc(&doc);

	setPreset(hDoc.ToNode(), true, presetIndex);
}

std::string MpController::getPresetXml()
{
	// Sort for export consistancy.
	list<SeParameter_vst3*> sortedParameters;
	for (auto& p : parameters_)
	{
		if (p->stateful_)
		{
			sortedParameters.push_back(p.get());
		}
	}
	sortedParameters.sort([](SeParameter_vst3* a, SeParameter_vst3* b) -> bool
	{
		if (a->getNativeParamterIndex() != a->getNativeParamterIndex())
			return a->getNativeParamterIndex() < b->getNativeParamterIndex();

		return a->parameterHandle_ < b->parameterHandle_;
	});

	TiXmlDocument doc;
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);

	auto element = new TiXmlElement("Preset");
	doc.LinkEndChild(element);
	// not much point: element->SetAttribute("Name", "Preset");

	for (auto parameter : sortedParameters)
	{
		auto paramElement = new TiXmlElement("Param");
		element->LinkEndChild(paramElement);
		paramElement->SetAttribute("id", parameter->parameterHandle_);

		const int voice = 0;
		auto raw = parameter->getValueRaw(FT_VALUE, voice);
		// nope turns blanks into double apostrophes:		string val = QuoteStringIfSpaces(RawToUtf8B(parameter->datatype_, raw.data(), raw.size()));
		string val = RawToUtf8B(parameter->datatype_, raw.data(), raw.size());

		//		if (size <= maxXmlAttributeBytes)
		paramElement->SetAttribute("val", val);
	}

	TiXmlPrinter printer;
//	printer.SetIndent(" ");
	doc.Accept(&printer);

	return printer.CStr();
}

int32_t MpController::getParameterModuleAndParamId(int32_t parameterHandle, int32_t* returnModuleHandle, int32_t* returnModuleParameterId)
{
	auto it = ParameterHandleIndex.find(parameterHandle);
	if (it != ParameterHandleIndex.end())
	{
		auto seParameter = (*it).second;
		*returnModuleHandle = seParameter->moduleHandle_;
		*returnModuleParameterId = seParameter->moduleParamId_;
		return gmpi::MP_OK;
	}
	return gmpi::MP_FAIL;
}

RawView MpController::getParameterValue(int32_t parameterHandle, int32_t moduleFieldId, int32_t voice)
{
	auto it = ParameterHandleIndex.find(parameterHandle);
	if (it != ParameterHandleIndex.end())
	{
		auto param = (*it).second;
		return param->getValueRaw(FT_VALUE, 0);
	}

	return RawView();
}

void MpController::setPreset(TiXmlNode* parentXml, bool updateProcessor, int preset)
{
	TiXmlNode* presetXml = nullptr;
	auto presetsXml = parentXml->FirstChild("Presets");
	if (presetsXml) // exported from SE has Presets/Preset
	{
		int presetIndex = 0;
		for (auto node = presetsXml->FirstChild("Preset"); node; node = node->NextSibling("Preset"))
		{
			presetXml = node;
			if (presetIndex++ >= preset)
				break;
		}
	}
	else
	{
		// Individual preset has "Preset" element.
		presetXml = parentXml->FirstChild("Preset");
	}

	TiXmlNode* parametersE = nullptr;
	if (presetXml)
	{
		// Individual preset has Preset.Param.val
		parametersE = presetXml; //  presetXml->FirstChild("Param eters")->ToElement();
	}
	else
	{
		auto controllerE = parentXml->FirstChild("Controller");

		// should always have a valid root but handle gracefully if it does
		if (!controllerE)
			return;

		auto patchManagerE = controllerE->FirstChildElement();
		assert(strcmp(patchManagerE->Value(), "PatchManager") == 0);
//		setPreset(patchManagerE, true, preset);

		// Internal preset has parameters wrapping preset values. (PatchManager.Parameters.Parameter.patch-list)
		parametersE = patchManagerE->FirstChild("Parameters")->ToElement();

		// Could be either of two formats. Internal (PatchManager.Parameter.patch-list), or vstpreset V1.3 (Parameters.Param)
		auto ParamElement13 = parametersE->FirstChildElement("Param"); /// is this a V1.3 vstpreset?
		if (ParamElement13)
		{
			presetXml = parametersE;
		}
	}

	if (parametersE == nullptr)
		return;

	if(updateProcessor)
		OnStartPresetChange();

	if (presetXml)
	{
		// assuming we are passed "Preset.Parameters" node.
		for (auto node = presetXml->FirstChild("Param"); node; node = node->NextSibling("Param"))
		{
			// check for existing
			auto ParamElement = node->ToElement();

			int paramHandle = -1;
			const int voiceId = 0;
			ParamElement->QueryIntAttribute("id", &paramHandle);
			if (paramHandle != -1)
			{
				auto it = ParameterHandleIndex.find(paramHandle);
				if (it != ParameterHandleIndex.end())
				{
					auto& parameter = (*it).second;
					if (parameter->stateful_) // For VST2 wrapper aeffect ptr. prevents it being inadvertantly zeroed.
					{
						std::string v = ParamElement->Attribute("val");

						auto raw = ParseToRaw(parameter->datatype_, v);

						parameter->setParameterRaw(FT_VALUE, (int32_t)raw.size(), raw.data(), voiceId);

						// updated cached value.
						parameter->upDateImmediateValue();

						if (updateProcessor)
							parameter->updateProcessor(FT_VALUE, voiceId);
					}
				}
			}
		}
	}
	else // Old-style. 'Internal' Presets
	{
		// assuming we are passed "PatchManager.Parameters" node.
		for (auto ParamElement = parametersE->FirstChildElement("Parameter"); ParamElement; ParamElement = ParamElement->NextSiblingElement("Parameter"))
		{
			// VST3 "Controler" XML uses "Handle", VST3 presets use "id" for the same purpose.
			int paramHandle = -1;
			ParamElement->QueryIntAttribute("Handle", &paramHandle);

			auto it = ParameterHandleIndex.find(paramHandle);
			if (it != ParameterHandleIndex.end())
			{
				auto& parameter = (*it).second;

				// Preset values from patch list.
				auto patch_xml = ParamElement->FirstChildElement("patch-list");
				if (patch_xml)
				{
					int voiceId = 0;
					for (; patch_xml; patch_xml = patch_xml->NextSiblingElement()) // for each voice.
					{
						std::vector<std::string> presetValues;
						XmlSplitString(patch_xml->GetText(), presetValues);

						int p = (std::min)(preset, (int)presetValues.size() - 1);
						auto raw = ParseToRaw(parameter->datatype_, presetValues[p]);

						if (parameter->setParameterRaw(FT_VALUE, (int32_t)raw.size(), raw.data(), voiceId))
						{
							// updated cached value.
							parameter->upDateImmediateValue();
							parameter->updateProcessor(FT_VALUE, voiceId);
						}

						++voiceId;
					}
				}
			}
		}
	}

	if (updateProcessor)
		OnEndPresetChange();
}

void MpController::LoadInternalPreset(int preset)
{
	// Load parameters XML. Contains all presets.
	auto xml = BundleInfo::instance()->getResource("parameters.se.xml");
	setPreset(xml, true, preset);
}

void MpController::LoadNativePresetFile(std::string presetName)
{
	auto xml = loadNativePreset( Utf8ToWstring(presetName) );
	setPreset(xml);
}

// xml may contain multiple presets, or just one.
void MpController::setPreset(const std::string& xml, bool updateProcessor, int preset)
{
	TiXmlDocument doc;
	doc.Parse(xml.c_str());

	if (doc.Error())
	{
		assert(false);
		return;
	}

	setPreset(&doc, updateProcessor, preset);
}

// Note: Don't handle polyphic stateful parameters.
void MpController::ExportPresetXml(const char* filename)
{
	ofstream myfile;
	myfile.open(filename);
	myfile << getPresetXml();
	myfile.close();
}

/*
std::wstring MpController::getNativePresetsList()
{
	auto vst3PresetFolder = BundleInfo::instance()->getPresetFolder();

	std::wostringstream oss;

	auto srcFolder = BundleInfo::instance()->getPresetFolder() + L"/*.";
	auto searchString = srcFolder + getNativePresetExtension();
	for (FileFinder it = searchString.c_str(); !it.done(); ++it)
	{
		if (!(*it).isFolder)
		{
			auto sourceFilename = combine_path_and_file(vst3PresetFolder, (*it).filename);

			if (first)
			{
				first = false;
			}
			else
			{
				oss << ',';
			}

			auto presetName = (*it).filename;

			// chop off extension
			auto p = presetName.find_last_of(L'.');
			if (p != std::string::npos)
				presetName = presetName.substr(0, p);

			oss << presetName;
		}
	}

	return oss.str();
}
*/

void MpController::ExportBankXml(const char* filename)
{
	// Create output XML document.
	tinyxml2::XMLDocument xml;
	xml.LinkEndChild(xml.NewDeclaration());

	auto presets_xml = xml.NewElement("Presets");
	xml.LinkEndChild(presets_xml);

	// Iterate native preset files, combine them into bank, and export.
	auto srcFolder = ToPlatformString(BundleInfo::instance()->getPresetFolder());
	auto searchString = srcFolder + _T("*.");
	searchString += ToPlatformString(getNativePresetExtension());
	for (FileFinder it = searchString.c_str(); !it.done(); ++it)
	{
		if (!(*it).isFolder)
		{
			auto sourceFilename = combine_path_and_file(srcFolder, (*it).filename);

			auto presetName = (*it).filename;
			{
				// chop off extension
				auto p = presetName.find_last_of(L'.');
				if (p != std::string::npos)
					presetName = presetName.substr(0, p);
			}

			auto chunk = loadNativePreset( ToWstring(sourceFilename) );

			//auto preset_xml = xml.NewElement("Preset");
			//presets_xml->LinkEndChild(preset_xml);
			//preset_xml->SetAttribute("Name", ToUtf8String(presetName).c_str());

			{
				tinyxml2::XMLDocument presetDoc;

				presetDoc.Parse(chunk.c_str());

				if (!presetDoc.Error())
				{
					auto parameters = presetDoc.FirstChildElement("Preset");
					auto copyOfParameters = parameters->DeepClone(&xml)->ToElement();
					presets_xml->LinkEndChild(copyOfParameters);
					copyOfParameters->SetAttribute("Name", ToUtf8String(presetName).c_str());
				}
			}
		}
	}

	// Save output XML document.
	xml.SaveFile(filename);
}

void MpController::ImportBankXml(const char* filename)
{
	auto presetFolder = BundleInfo::instance()->getPresetFolder();

	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename);

	if (doc.Error())
	{
		assert(false);
		return;
	}

	auto presetsE = doc.FirstChildElement("Presets");

	for (auto PresetE = presetsE->FirstChildElement("Preset"); PresetE; PresetE = PresetE->NextSiblingElement())
	{
		auto name = PresetE->Attribute("Name");
		auto filename = presetFolder + Utf8ToWstring(name) + L".";
		filename += getNativePresetExtension();

		// dialog if file exists.
		auto result = gmpi::MP_OK;

/* no mac support
		fs::path fn(filename);
		if (fs::exists(fn))
*/
#ifdef UNICODE
        auto file = _wfopen(filename.c_str(), L"r"); // fs::exists(filename)
#else
        auto file = fopen(WStringToUtf8(filename).c_str(), "r");
#endif
        if(file)
		{
			fclose(file);

			std::ostringstream oss;
			oss << "Overwrite preset '" << name << "'?";
            
#ifdef _WIN32
			auto buttons = MB_YESNOCANCEL; // MB_RETRYCANCEL MB_YESNO MB_YESNOCANCEL MB_ABORTRETRYIGNORE MB_CANCELTRYCONTINUE MB_OK
			HWND parentWnd = 0;
			auto r = MessageBoxA(parentWnd, oss.str().c_str(), "Preset exists", buttons);

			if (r == IDCANCEL)
				return;

			result = r == IDYES ? gmpi::MP_OK : gmpi::MP_CANCEL;
#endif
		}

		if (result == gmpi::MP_OK)
		{
			tinyxml2::XMLPrinter printer;
			PresetE->Accept(&printer);
			std::string presetXml(printer.CStr(), (size_t)printer.CStrSize());

			saveNativePreset( WStringToUtf8(filename).c_str(), presetXml);
		}
	}
}
