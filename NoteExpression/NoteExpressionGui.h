#ifndef NOTEEXPRESSIONGUI_H_INCLUDED
#define NOTEEXPRESSIONGUI_H_INCLUDED

#include "MP_SDK_GUI.h"

class NoteExpressionGui : public MpGuiBase
{
public:
	NoteExpressionGui( IMpUnknown* host );

	// overrides

private:
 	void onSetProgram();
 	void onSetProgramNamesList();
 	void onSetProgramName();
 	void onSetMidiChannel();
 	void onSetPatchCommand();
 	void onSetProgramOut();
 	void onSetProgramNamesListOut();
 	void onSetProgramNameOut();
 	void onSetChannel();
 	void onSetChannelList();
 	void onSetPatchCommandOut();
 	void onSetPatchCommandsList();
 	void onSetPatchCommandList();
 	IntGuiPin pinProgram;
 	StringGuiPin pinProgramNamesList;
 	StringGuiPin pinProgramName;
 	IntGuiPin pinMidiChannel;
 	IntGuiPin pinPatchCommand;
 	IntGuiPin pinProgramOut;
 	StringGuiPin pinProgramNamesListOut;
 	StringGuiPin pinProgramNameOut;
 	IntGuiPin pinChannel;
 	StringGuiPin pinChannelList;
 	IntGuiPin pinPatchCommandOut;
 	StringGuiPin pinPatchCommandsList;
 	StringGuiPin pinPatchCommandList;
};

#endif


