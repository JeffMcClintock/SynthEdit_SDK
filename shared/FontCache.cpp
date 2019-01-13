#include "FontCache.h"
#include "../shared/string_utilities.h"
#include "../shared/it_enum_list.h"
#include "../shared/string_utilities.h"
#include "../se_sdk3/MpString.h"

using namespace std;
using namespace gmpi;
using namespace gmpi_gui;
using namespace gmpi_sdk;
using namespace GmpiDrawing;

FontCache::FontCache() :
	clientCount_(0)
{
}

FontCache* FontCache::instance()
{
	static FontCache instance;
	return &instance;
}

GmpiDrawing::TextFormat FontCacheClient::GetTextFormat(std::string style)
{
	return GetTextFormat(dynamic_cast<MpGuiBase2*>(this)->getHost(), dynamic_cast<MpGuiGfxBase*>(this)->getGuiHost(), style);
}

FontMetadata* FontCacheClient::GetFontMetatdata(std::string style)
{
	FontMetadata* returnFontData = nullptr;
	GetTextFormat(dynamic_cast<MpGuiBase2*>(this)->getHost(), dynamic_cast<MpGuiGfxBase*>(this)->getGuiHost(), style, &returnFontData);
	return returnFontData;
}

GmpiDrawing::TextFormat FontCache::GetTextFormat(gmpi::IMpUserInterfaceHost2* host, gmpi_gui::IMpGraphicsHost* guiHost, std::string style, FontMetadata** returnMetadata)
{
	if (returnMetadata)
		*returnMetadata = nullptr;

	Factory factory;
	guiHost->GetDrawingFactory(factory.GetAddressOf());

	// Get my skin URI.
	MpString fullUri;
	host->RegisterResourceUri("global", "ImageMeta", &fullUri);

	for (auto& cachedfont : fonts_)
	{
		if (cachedfont.factory == factory.Get() && cachedfont.skinId == fullUri.str() && cachedfont.style == style)
		{
			if (returnMetadata)
			{
				*returnMetadata = cachedfont.metadata.get();
			}

			return cachedfont.textFormat;
			break;
		}
	} 

	// Load skin text file
	SkinMetadata* skin1 = nullptr;

	for (auto& cachedskin : skins_)
	{
		if (cachedskin.skinUri == fullUri.str())
		{
			skin1 = &cachedskin;
			break;
		}
	}

	if (skin1 == nullptr)
	{
		gmpi_sdk::mp_shared_ptr<gmpi::IProtectedFile2> stream2;
		host->OpenUri(fullUri.c_str(), stream2.getAddressOf());
		if (stream2 == 0)
		{
			return factory.CreateTextFormat(); // in case of failure, return a default textformat.
		}

		skins_.push_back(SkinMetadata());
		skins_.back().skinUri = fullUri.str();
		skins_.back().Serialise(stream2);
		skin1 = &(skins_.back());
	}

	auto fontmetadata = skin1->getFont(style);

	auto textFormat = factory.CreateTextFormat(
		(float)fontmetadata->size_,
		fontmetadata->faceFamilies_[0].c_str(),
		(GmpiDrawing_API::MP1_FONT_WEIGHT) fontmetadata->getWeight(),
		(GmpiDrawing_API::MP1_FONT_STYLE) fontmetadata->getStyle());

	fonts_.push_back(TypefaceData(factory.Get(), fullUri.str(), style, textFormat, fontmetadata));

	if (returnMetadata)
		*returnMetadata = fonts_.back().metadata.get();

	return textFormat;
}

void FontCache::RegisterCustomTextFormat(gmpi::IMpUserInterfaceHost2* host, gmpi_gui::IMpGraphicsHost* guiHost, std::string style, const FontMetadata* fontmetadata)
{
	Factory factory;
	guiHost->GetDrawingFactory(factory.GetAddressOf());

	// Get my skin URI.
	MpString fullUri;
	host->RegisterResourceUri("global", "ImageMeta", &fullUri);

	// Create font.
	GmpiDrawing::TextFormat textFormat;
	factory.Get()->CreateTextFormat(
		fontmetadata->faceFamilies_[0].c_str(),
		NULL,
		(GmpiDrawing_API::MP1_FONT_WEIGHT) fontmetadata->getWeight(),
		(GmpiDrawing_API::MP1_FONT_STYLE) fontmetadata->getStyle(),
		GmpiDrawing_API::MP1_FONT_STRETCH_NORMAL,
		(float)fontmetadata->size_, // dipFontSize
		0,							//locale
		textFormat.GetAddressOf()
	);

	fonts_.push_back(TypefaceData(factory.Get(), fullUri.str(), style, textFormat, fontmetadata));
	fonts_.back().style = style; // custom style name.
}

// Need to keep track of clients so imagecache can be cleared BEFORE program exit (else WPF crashes).
void FontCache::RemoveClient()
{
	if (--clientCount_ == 0)
	{
		fonts_.clear();
	}
}

