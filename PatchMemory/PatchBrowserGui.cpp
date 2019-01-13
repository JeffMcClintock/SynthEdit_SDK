#include "../se_sdk3/mp_sdk_gui2.h"
#include <sstream>
//#include <filesystem>
#include "../shared/FileFinder.h"
#include "../shared/it_enum_list.h"
#include "../shared/unicode_conversion.h"

using namespace gmpi;
using namespace JmUnicodeConversions;

//namespace fs = std::experimental::filesystem::v1;

class PatchBrowserGui : public SeGuiInvisibleBase
{
	IntGuiPin programIn;
	StringGuiPin programNamesListIn;
//	StringGuiPin programCategoriesListIn;
	IntGuiPin patchCommandIn;
	StringGuiPin patchCommandListIn;

 	IntGuiPin pinPreset;
 	StringGuiPin pinPresetNamesList;
	IntGuiPin patchCommandOut;
	StringGuiPin patchCommandListOut;
    
    platform_string presetExtension()
    {
#ifdef _WIN32
        return _T("vstpreset");
#else
        return _T("aupreset");
#endif
    }

	std::string getVst3PresetsFolder()
	{
		gmpi_sdk::MpString vst3PresetFolderString;
		getHost()->FindResourceU("__nativePresetsFolder", "", &vst3PresetFolderString);
		return vst3PresetFolderString.str();
	}

	std::wstring getNativePresetsList()
	{
		platform_string PresetFolder = toPlatformString(getVst3PresetsFolder());

		auto extension = presetExtension();
		const auto searchString = PresetFolder + platform_string(_T("*.")) + extension;

		std::wostringstream oss;
		FileFinder it(searchString.c_str());
		bool first = true;
		for (; !it.done(); ++it)
		{
			if (!(*it).isFolder)
			{
				auto sourceFilename = PresetFolder + (*it).filename;
				
				if (first)
				{
					first = false;
				}
				else
				{
					oss << L',';
				}

				auto presetName = (*it).filename;

				// chop off extension
				auto p = presetName.find(extension);
				if (p != std::string::npos)
					presetName = presetName.substr(0, p - 1);

				oss << toWstring(presetName);
			}
		}

		return oss.str();
	}

	void onSetPreset()
	{
		if (pinPreset >= 0)
		{
			it_enum_list it(pinPresetNamesList);

			if (it.FindIndex(pinPreset))
			{
				LoadNativePresetFile(it.CurrentItem()->text);
			}
		}
	}

	void LoadNativePresetFile(std::wstring presetName)
	{
		//fs::path vst3PresetFolder(getVst3PresetsFolder());
		//auto fn = vst3PresetFolder / presetName;
		//fn.replace_extension(".vstpreset");

		platform_string vst3PresetFolder = toPlatformString(getVst3PresetsFolder());
		auto fn = vst3PresetFolder + toPlatformString(presetName) + toPlatformString(_T(".")) + presetExtension();

//		getHost()->LoadPresetFile(fn.generic_u8string().c_str());
		getHost()->LoadPresetFile(toString(fn).c_str());
	}

public:
	PatchBrowserGui()
	{
		// Currently ignoring internal preset list.
		// Might be good to enable it in Edit mode.
		initializePin(programIn, static_cast<MpGuiBaseMemberPtr2>(&PatchBrowserGui::onSetprogramIn));
		initializePin(programNamesListIn, static_cast<MpGuiBaseMemberPtr2>(&PatchBrowserGui::onSetprogramNamesListIn));
		initializePin(patchCommandIn);
		initializePin(patchCommandListIn, static_cast<MpGuiBaseMemberPtr2>(&PatchBrowserGui::onSetPatchCommandListIn));

		initializePin( pinPreset, static_cast<MpGuiBaseMemberPtr2>(&PatchBrowserGui::onSetPreset) );
		initializePin( pinPresetNamesList );
		initializePin(patchCommandOut, static_cast<MpGuiBaseMemberPtr2>(&PatchBrowserGui::onSetpatchCommandOut));
		initializePin(patchCommandListOut);

//		initializePin(programCategoriesListIn);
		
//		add pins for patch commands, refresh presets on import bank
//			add factory support and bool to enable/disable it
	}

	void onSetprogramIn()
	{
//		programOut = programIn;
	}

	void onSetprogramNamesListIn()
	{
//		programNamesList = programNamesListIn;
		pinPresetNamesList = getNativePresetsList();
	}

	void onSetpatchCommandOut()
	{
		patchCommandIn = patchCommandOut;

		if (patchCommandOut == 4) // "Import Bank". magic number.
		{
			// Refresh disk preset list.
			onSetprogramNamesListIn();
		}

		// Reset to Zero after executing command.
		if (patchCommandOut > 0)
		{
			patchCommandOut = 0;
			patchCommandIn = 0;
		}
	}

	void onSetPatchCommandListIn()
	{
		patchCommandListOut = patchCommandListIn;
	}
};

namespace
{
	auto r = Register<PatchBrowserGui>::withId(L"SE Patch Browser");
}
