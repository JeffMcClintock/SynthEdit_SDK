#include ".\GuiComTest2Gui.h"


REGISTER_GUI_PLUGIN( GuiComTest2Gui, L"SE GUI COM Test2" );

GuiComTest2Gui::GuiComTest2Gui(IMpUnknown* host) : SeGuiWindowsGfxBase(host)
{
	// initialise pins
//	captureDataA.initialize( this, 0, static_cast<MpGuiBaseMemberPtr>(&GuiComTest2Gui::onSetCaptureDataA) );
//	captureDataB.initialize( this, 1, static_cast<MpGuiBaseMemberPtr>(&GuiComTest2Gui::onSetCaptureDataB) );
}

// handle pin updates
void GuiComTest2Gui::onSetCaptureDataA()
{
	// captureDataA changed
}

void GuiComTest2Gui::onSetCaptureDataB()
{
	// captureDataB changed
}

void GuiComTest2Gui::onSetVoiceGate()
{
	// voiceGate changed
}

void GuiComTest2Gui::onSetpolydetect()
{
	// polydetect changed
}


int32_t GuiComTest2Gui::onLButtonDown(UINT flags, POINT point)
{
	getHost()->sendMessageToAudio( 23, 0, 0 );
	return gmpi::MP_OK;
}