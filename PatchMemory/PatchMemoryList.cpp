#include "./PatchMemoryList.h"
//#include "./PatchMemoryList.xml.h"

REGISTER_PLUGIN( PatchMemoryList, L"SE PatchMemory List3" );
//REGISTER_XML( PATCHMEMORYLIST_XML );

PatchMemoryList::PatchMemoryList(IMpUnknown* host) : MpBase(host)
{
	initializePin( pinValueIn );
	initializePin( pinValueOut );
}

void PatchMemoryList::onSetPins(void)  // one or more pins_ updated.  Check pin update flags to determin which ones.
{
	pinValueOut = (int) pinValueIn;
}







