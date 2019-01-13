#ifndef GUICOMTEST2_H_INCLUDED
#define GUICOMTEST2_H_INCLUDED

#include "mp_sdk_audio.h"

class GuiComTest2 : public MpBase
{
public:
	GuiComTest2( IMpUnknown* host );
	virtual void onSetPins(void);
	virtual int32_t MP_STDCALL recieveMessageFromGui( int32_t id, int32_t size, void* messageData );

private:
	AudioInPin pinIn;
	AudioOutPin pinOut;
};

#endif

