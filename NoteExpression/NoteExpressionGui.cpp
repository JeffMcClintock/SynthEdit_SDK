#include ".\NoteExpressionGui.h"


REGISTER_GUI_PLUGIN( NoteExpressionGui, L"SE Note Expression" );

NoteExpressionGui::NoteExpressionGui( IMpUnknown* host ) : MpGuiBase(host)
{
	// initialise pins.
	pinProgram.initialize( this, 0, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetProgram) );
	pinProgramNamesList.initialize( this, 1, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetProgramNamesList) );
	pinProgramName.initialize( this, 2, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetProgramName) );
	pinMidiChannel.initialize( this, 3, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetMidiChannel) );
	pinPatchCommand.initialize( this, 4, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetPatchCommand) );
	pinProgramOut.initialize( this, 5, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetProgramOut) );
	pinProgramNamesListOut.initialize( this, 6, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetProgramNamesListOut) );
	pinProgramNameOut.initialize( this, 7, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetProgramNameOut) );
	pinChannel.initialize( this, 8, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetChannel) );
	pinChannelList.initialize( this, 9, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetChannelList) );
	pinPatchCommandOut.initialize( this, 10, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetPatchCommandOut) );
	pinPatchCommandsList.initialize( this, 11, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetPatchCommandsList) );
	pinPatchCommandList.initialize( this, 12, static_cast<MpGuiBaseMemberPtr>(&NoteExpressionGui::onSetPatchCommandList) );
}

// handle pin updates.
void NoteExpressionGui::onSetProgram()
{
	// pinProgram changed
}

void NoteExpressionGui::onSetProgramNamesList()
{
	// pinProgramNamesList changed
}

void NoteExpressionGui::onSetProgramName()
{
	// pinProgramName changed
}

void NoteExpressionGui::onSetMidiChannel()
{
	// pinMidiChannel changed
}

void NoteExpressionGui::onSetPatchCommand()
{
	// pinPatchCommand changed
}

void NoteExpressionGui::onSetProgramOut()
{
	// pinProgramOut changed
}

void NoteExpressionGui::onSetProgramNamesListOut()
{
	// pinProgramNamesListOut changed
}

void NoteExpressionGui::onSetProgramNameOut()
{
	// pinProgramNameOut changed
}

void NoteExpressionGui::onSetChannel()
{
	// pinChannel changed
}

void NoteExpressionGui::onSetChannelList()
{
	// pinChannelList changed
}

void NoteExpressionGui::onSetPatchCommandOut()
{
	// pinPatchCommandOut changed
}

void NoteExpressionGui::onSetPatchCommandsList()
{
	// pinPatchCommandsList changed
}

void NoteExpressionGui::onSetPatchCommandList()
{
	// pinPatchCommandList changed
}

