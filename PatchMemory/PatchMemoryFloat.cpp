#include "./PatchMemoryFloat.h"

REGISTER_PLUGIN( PatchMemoryFloat, L"SE PatchMemory Float" );
REGISTER_PLUGIN( PatchMemoryFloat, L"SE PatchMemory Float Out" );
REGISTER_PLUGIN( PatchMemoryFloat, L"SE PatchMemory Float Out B2" );

PatchMemoryFloat::PatchMemoryFloat(IMpUnknown* host) : MpBase(host)
{
	initializePin( 0, pinValueIn );
	initializePin( 1, pinValueOut );
}

void PatchMemoryFloat::onSetPins(void)  // one or more pins_ updated.  Check pin update flags to determin which ones.
{
	if( pinValueIn.isUpdated() )
	{
		pinValueOut = pinValueIn;
	}
}







