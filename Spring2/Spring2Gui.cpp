#include "./Spring2Gui.h"

REGISTER_GUI_PLUGIN( Spring2Gui, L"SE Spring2" );
REGISTER_GUI_PLUGIN( Spring2Gui, L"SE Spring" );

Spring2Gui::Spring2Gui(IMpUnknown* host) : MpGuiBase(host)
,prevMouseDown( false )
{
	// initialise pins
	normalisedValue.initialize( this, 0 );
	mouseDown.initialize( this, 1, static_cast<MpGuiBaseMemberPtr>(&Spring2Gui::onSetMouseDown) );
	resetValue.initialize(this, 2);
	enable.initialize(this, 3, static_cast<MpGuiBaseMemberPtr>(&Spring2Gui::onEnabled));
}

void Spring2Gui::onSetMouseDown()
{
	// mouseDown changed
	if(enable && mouseDown == false && prevMouseDown == true )
	{
		normalisedValue = resetValue;
	}

	prevMouseDown = mouseDown;
}

void Spring2Gui::onEnabled()
{
	// mouseDown changed
	if (enable && mouseDown == false)
	{
		normalisedValue = resetValue;
	}
}