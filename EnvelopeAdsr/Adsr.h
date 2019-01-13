#ifndef ADSRDSP_H_INCLUDED
#define ADSRDSP_H_INCLUDED

#include "Envelope.h"

class Adsr : public Envelope
{
public:
	Adsr( IMpUnknown* host );

private:
	AudioInPin pinAttack;
	AudioInPin pinDecay;
	AudioInPin pinSustain;
	AudioInPin pinRelease;
};

#endif

