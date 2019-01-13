#pragma once
#include <vector>
#include "../se_sdk3/mp_sdk_stdint.h"
#include "RawView.h"

namespace SynthEdit2
{
	// kind of a patch-cable-list deserialiser.
	class PatchCables
	{
		struct Cable
		{
			int32_t fromUgHandle;
			int32_t toUgHandle;
			int32_t fromUgPin;
			int32_t toUgPin;
		};

	public:
		std::vector< Cable > cables;

		PatchCables(RawView raw);
		RawData Serialise();

		void insert(int32_t fromModule, int fromPin, int32_t toModule, int toPin)
		{
			auto it = cables.insert(cables.begin(), PatchCables::Cable());
			auto& c = *it;

			c.fromUgHandle = fromModule;
			c.toUgHandle = toModule;
			c.fromUgPin = fromPin;
			c.toUgPin = toPin;
		}

		void push_back(int32_t fromModule, int fromPin, int32_t toModule, int toPin)
		{
			cables.push_back(PatchCables::Cable());
			auto& c = cables.back();
			c.fromUgHandle = fromModule;
			c.toUgHandle = toModule;
			c.fromUgPin = fromPin;
			c.toUgPin = toPin;
		}
	};
}