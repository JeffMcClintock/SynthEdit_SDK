#include "./ClassicControlGuiBase.h"
#include "TextWidget.h"

void ClassicControlGuiBase::onSetTitle()
{
	if (!widgets.empty())
	{
		auto header = dynamic_cast<TextWidget*>(widgets.back().get());

		header->SetText(pinTitle);

		if (header->ClearDirty())
			invalidateMeasure();
	}
}

int32_t ClassicControlGuiBase::initialize()
{
	auto r = gmpi_gui::MpGuiGfxBase::initialize(); // ensure all pins initialised (so widgets are built).

	dynamic_cast<TextWidget*>(widgets.back().get())->SetText(pinTitle);

	return r;
}
