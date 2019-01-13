#ifndef DBTOANIMATIONGUI_H_INCLUDED
#define DBTOANIMATIONGUI_H_INCLUDED

#include "MP_SDK_GUI.h"

class DbToAnimationGui : public MpGuiBase
{
public:
	DbToAnimationGui( IMpUnknown* host );

	// overrides

private:
 	void onSetAnimationPosition();
 	void onSetDb();
 	FloatGuiPin animationPosition;
 	FloatGuiPin db;
};

#endif


