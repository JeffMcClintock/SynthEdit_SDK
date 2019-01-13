#ifndef USERSETTINGTEXT_H_INCLUDED
#define USERSETTINGTEXT_H_INCLUDED

#include "mp_sdk_audio.h"

class UserSettingText : public MpBase
{
public:
	UserSettingText( IMpUnknown* host );

private:
	StringOutPin pinValueOut;
};

#endif

