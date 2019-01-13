#include ".\UserSettingText.h"

REGISTER_PLUGIN ( UserSettingText, L"SE UserSettingText" );

UserSettingText::UserSettingText( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, pinValueOut );
}


