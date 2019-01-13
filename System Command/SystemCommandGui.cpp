#include "./SystemCommandGui.h"
#include "../shared/xplatform.h"


REGISTER_GUI_PLUGIN( SystemCommandGui, L"SE System Command" );

SystemCommandGui::SystemCommandGui( IMpUnknown* host ) : MpGuiBase( host )
,previousTrigger( false )
{
	// initialise pins.
	trigger.initialize( this, 0, static_cast<MpGuiBaseMemberPtr>(&SystemCommandGui::onSetTrigger) );
	command.initialize( this, 1 );
	commandList.initialize( this, 2 );
	filename.initialize( this, 3 );
}

// handle pin updates.
void SystemCommandGui::onSetTrigger()
{
	if( trigger == false && previousTrigger == true )
	{
		wchar_t fullFilename[MAX_PATH];

		getHost()->resolveFilename( filename.getValue().c_str(), MAX_PATH, fullFilename );

		if( command < 0 || command > 5 )
		{
			return;
		}
#ifdef _WIN32
		const wchar_t* commands[] = { L"edit", L"explore", L"find", L"open", L"print", L"properties" };
		ShellExecute( 0, commands[command], fullFilename, L"", L"", SW_MAXIMIZE );
#else
        // TODO!! Mac equivalent.
#endif
	}
	previousTrigger = trigger;
}

int32_t SystemCommandGui::initialize()
{
	commandList = L"edit,explore,find,open,print,properties";

	return MpGuiBase::initialize();
}

