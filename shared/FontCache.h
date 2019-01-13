#pragma once
#include <map>
#include <string>
#include <memory>
#include "../se_sdk3/mp_sdk_gui2.h"
#include "../shared/ImageMetadata.h"
/* 
#include "../shared/FontCache.h"
*/

struct TypefaceData
{
	TypefaceData(GmpiDrawing_API::IMpFactory* pfactory, std::string pskinId, std::string pstyle, GmpiDrawing::TextFormat ptextFormat, const FontMetadata* pmetadata) :
		factory(pfactory)
		, style(pstyle)
		, skinId(pskinId)
		, textFormat(ptextFormat)
	{
		metadata = std::make_unique<FontMetadata>(*pmetadata);
	}

	GmpiDrawing_API::IMpFactory* factory; // differentiates between GDI and D2D bitmaps.
	std::string skinId;
	std::string style;
	GmpiDrawing::TextFormat textFormat;
	std::unique_ptr<FontMetadata> metadata;
};

class FontCache
{
	friend class FontCacheClient;
	FontCache();

	std::vector<TypefaceData> fonts_;
	std::vector<SkinMetadata> skins_;

	int clientCount_;
protected:
	
	// Need to keep track of clients so imagecache can be cleared BEFORE program exit (else WPF crashes).
	void AddClient() {
		++clientCount_;
	}
	void RemoveClient();

public:
	static FontCache* instance();

	GmpiDrawing::TextFormat GetTextFormat(gmpi::IMpUserInterfaceHost2* host, gmpi_gui::IMpGraphicsHost* guiHost, std::string style, FontMetadata** metadata = 0);
	void RegisterCustomTextFormat(gmpi::IMpUserInterfaceHost2* host, gmpi_gui::IMpGraphicsHost* guiHost, std::string style, const FontMetadata* metadata );
};


class FontCacheClient
{
public:
	FontCacheClient()
	{
		FontCache::instance()->AddClient();
	}
	virtual ~FontCacheClient()
	{
		FontCache::instance()->RemoveClient();
	}

	GmpiDrawing::TextFormat GetTextFormat(gmpi::IMpUserInterfaceHost2* host, gmpi_gui::IMpGraphicsHost* guiHost, std::string style, FontMetadata** metadata = 0)
	{
		return FontCache::instance()->GetTextFormat(host, guiHost, style, metadata);
	}

	void RegisterCustomTextFormat(gmpi::IMpUserInterfaceHost2* host, gmpi_gui::IMpGraphicsHost* guiHost, std::string style, const FontMetadata* metadata)
	{
		return FontCache::instance()->RegisterCustomTextFormat(host, guiHost, style, metadata);
	}

	GmpiDrawing::TextFormat GetTextFormat(std::string style);
	FontMetadata* GetFontMetatdata(std::string style);
};
