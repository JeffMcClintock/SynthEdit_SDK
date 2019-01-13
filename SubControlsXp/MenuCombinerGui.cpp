#include "mp_sdk_gui2.h"
#include "it_enum_list.h"

using namespace gmpi;

class MenuCombinerGui : public SeGuiInvisibleBase
{
  	StringGuiPin pinAItemList;
 	IntGuiPin pinAChoice;
 	StringGuiPin pinBItemList;
 	IntGuiPin pinBChoice;
 	StringGuiPin pinItemListOut;
 	IntGuiPin pinChoiceOut;
 	BoolGuiPin pinASupportMomentary;
 	BoolGuiPin pinBSuportMomentary;
	StringGuiPin pinASubmenu;
	StringGuiPin pinBSubmenu;

	int firstListStartIndex = 0;
	int secondListStartIndex = 0;

public:
	MenuCombinerGui()
	{
		initializePin( pinAChoice );
		initializePin( pinAItemList, static_cast<MpGuiBaseMemberPtr2>(&MenuCombinerGui::onSetItemListIn) );
		initializePin( pinBChoice );
		initializePin( pinBItemList, static_cast<MpGuiBaseMemberPtr2>(&MenuCombinerGui::onSetItemListIn) );
		initializePin( pinChoiceOut, static_cast<MpGuiBaseMemberPtr2>(&MenuCombinerGui::onSetChoice) );
		initializePin( pinItemListOut);
		initializePin(pinASupportMomentary);
		initializePin(pinBSuportMomentary);
		initializePin(pinASubmenu, static_cast<MpGuiBaseMemberPtr2>(&MenuCombinerGui::onSetItemListIn));
		initializePin(pinBSubmenu, static_cast<MpGuiBaseMemberPtr2>(&MenuCombinerGui::onSetItemListIn));
	}

	void onSetItemListIn()
	{
		std::wstring combinedList;

		firstListStartIndex = 0;

		if (!pinASubmenu.getValue().empty())
		{
			combinedList.append(L">>>>");
			combinedList.append(pinASubmenu.getValue());
			++firstListStartIndex;
		}

		secondListStartIndex = firstListStartIndex;

		it_enum_list it(pinAItemList);
		for (it.First(); !it.IsDone(); it.Next())
		{
			enum_entry* e = it.CurrentItem();

			if (combinedList.empty())
			{
				combinedList.append(e->text);
				combinedList.append(L"=1");
			}
			else
			{
				combinedList.append(L",");
				combinedList.append(e->text);
			}
			++secondListStartIndex;
		}
		if (!pinASubmenu.getValue().empty())
		{
			if (!combinedList.empty())
			{
				combinedList.append(L",");
			}

			combinedList.append(L"<<<<");
			++secondListStartIndex;
		}

		// 2nd list
		if (!pinBSubmenu.getValue().empty())
		{
			if (!combinedList.empty())
			{
				combinedList.append(L",");
			}
			combinedList.append(L">>>>");
			combinedList.append(pinBSubmenu.getValue());
			++secondListStartIndex;
		}
		it_enum_list it2(pinBItemList);
		for (it2.First(); !it2.IsDone(); it2.Next())
		{
			enum_entry* e = it2.CurrentItem();
			if (combinedList.empty())
			{
				combinedList.append(e->text);
				combinedList.append(L"=1");
			}
			else
			{
				combinedList.append(L",");
				combinedList.append(e->text);
			}
		}
		if (!pinBSubmenu.getValue().empty())
		{
			if (!combinedList.empty())
			{
				combinedList.append(L",");
			}
			combinedList.append(L"<<<<");
		}

		pinItemListOut = combinedList;
	}

	void onSetChoice()
	{
		if (pinChoiceOut == 0)
		{
			if (pinASupportMomentary)
			{
				pinAChoice = 0;
			}
			if (pinBSuportMomentary)
			{
				pinBChoice = 0;
			}
			return;
		}

		it_enum_list it(pinAItemList);
		if (it.FindIndex(pinChoiceOut - firstListStartIndex))
		{
			pinAChoice = it.CurrentItem()->value;
		}
		else
		{
			it_enum_list it2(pinBItemList);
			if (it2.FindIndex(pinChoiceOut - secondListStartIndex))
			{
				pinBChoice = it2.CurrentItem()->value;
			}
		}
/*
		int indexOut = 1;

		it_enum_list it(pinAItemList);
		for (it.First(); !it.IsDone(); it.Next())
		{
			enum_entry* e = it.CurrentItem();
			if (indexOut == pinChoiceOut)
			{
				pinAChoice = e->value;
				return;
			}
			++indexOut;
		}

		it_enum_list it2(pinBItemList);
		for (it2.First(); !it2.IsDone(); it2.Next())
		{
			enum_entry* e = it2.CurrentItem();
			if (indexOut == pinChoiceOut)
			{
				pinBChoice = e->value;
				return;
			}
			++indexOut;
		}
		*/
	}
};

namespace
{
	auto r = Register<MenuCombinerGui>::withId(L"SE Menu Combiner");
}
