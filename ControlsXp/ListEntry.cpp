#include "./ListEntry.h"

REGISTER_PLUGIN2 ( ListEntry, L"SE List Entry" );

ListEntry::ListEntry()
{
	// Register pins.
	initializePin(pinValueIn);
	initializePin(pinValueOut);
}

void ListEntry::onSetPins(void)
{
	pinValueOut = pinValueIn;
}