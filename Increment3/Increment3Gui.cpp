#include "./Increment3Gui.h"
#include "it_enum_list.h"


REGISTER_GUI_PLUGIN(Increment3Gui, L"SynthEdit Increment3");
REGISTER_GUI_PLUGIN(Increment3Gui, L"SE Increment3B");

Increment3Gui::Increment3Gui(IMpUnknown* host) : MpGuiBase(host)
{
	// initialise pins
/* old
	choice.initialize( this, 0 );
	itemList.initialize( this, 1 );
	increment.initialize( this, 2, static_cast<MpGuiBaseMemberPtr>(&Increment3Gui::onSetIncrement) );
	decrement.initialize( this, 3, static_cast<MpGuiBaseMemberPtr>(&Increment3Gui::onSetDecrement) );
	wrap.initialize( this, 4 );
*/
	initializePin( 0, choice );
	initializePin( 1, itemList );
	initializePin( 2, increment, static_cast<MpGuiBaseMemberPtr>( &Increment3Gui::onSetIncrement ) );
	initializePin( 3, decrement, static_cast<MpGuiBaseMemberPtr>( &Increment3Gui::onSetDecrement ) );
	initializePin( 4, wrap );
}

void Increment3Gui::onSetIncrement()
{
	if(!increment)
	{
		nextValue( 1 );
	}
}

void Increment3Gui::onSetDecrement()
{
	if(!decrement)
	{
		nextValue( -1 );
	}
}

void Increment3Gui::nextValue( int direction )
{
	it_enum_list itr( itemList );
	itr.First();
	if( itr.IsDone() ) // empty list
		return;

	int first_idx = itr.CurrentItem()->value;
	int cur_idx = choice;
	int last_idx = first_idx;
	int prev_idx = first_idx;
	int next_idx = first_idx;
	bool found_current = false;

	for(  ; !itr.IsDone() ; itr.Next() )
	{
		enum_entry *e = itr.CurrentItem();
		last_idx = e->value;

		if( last_idx == cur_idx )
		{
			found_current = true;
			next_idx = cur_idx;
		}

		if( found_current && next_idx == cur_idx )
		{
			next_idx = last_idx;
		}

		if( !found_current )
		{
			prev_idx = last_idx;
		}
	}

	if( direction == 1  ) // increment
	{
		if( cur_idx == last_idx && wrap )
			next_idx = first_idx;

		choice = next_idx;
	}
	else // decrement
	{
		if( cur_idx == first_idx && wrap )
			prev_idx = last_idx;

		choice = prev_idx;
	}
}
