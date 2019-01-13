#ifndef USERSETTINGTEXTGUI_H_INCLUDED
#define USERSETTINGTEXTGUI_H_INCLUDED

#include "MP_SDK_GUI.h"

class UserSettingTextGui : public MpGuiBase
{
public:
	UserSettingTextGui( IMpUnknown* host );
	virtual int32_t MP_STDCALL initialize();

private:
	std::string getSettingFilePath();
	void onSetValue();

	StringGuiPin pinProduct;
	StringGuiPin pinKey;
	StringGuiPin pinDefault;
	StringGuiPin pinValue;
};

#endif


