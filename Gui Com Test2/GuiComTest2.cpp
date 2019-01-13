#include ".\GuiComTest2.h"

REGISTER_PLUGIN ( GuiComTest2, L"SE GUI COM Test2" );

GuiComTest2::GuiComTest2( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, pinIn );
	initializePin( 1, pinOut );
}

void GuiComTest2::onSetPins(void)
{
	pinOut.setStreaming( pinIn.isStreaming() );
}


int32_t MP_STDCALL GuiComTest2::recieveMessageFromGui( int32_t id, int32_t size, void* messageData )
{
	_RPT1(_CRT_WARN, "recieveMessageFromGui %d\n", this );
	return gmpi::MP_OK;
}
