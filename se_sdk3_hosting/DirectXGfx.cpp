#include <sstream>
#include "DirectXGfx.h"
#include "modules\shared\xplatform.h"
#include "modules/shared/xp_simd.h"
#include "modules/se_sdk3_hosting/gmpi_gui_hosting.h"
#include "modules/shared/fast_gamma.h"

using namespace GmpiGuiHosting;

#if defined(SE_SUPPORT_MFC) 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#endif

namespace gmpi
{
	namespace directx
	{
		int32_t Geometry::Open(GmpiDrawing_API::IMpGeometrySink** geometrySink)
		{
			ID2D1GeometrySink* sink = nullptr;

			auto hr = geometry_->Open(&sink);

			if (hr == 0)
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new gmpi::directx::GeometrySink(sink));

				b2->queryInterface(GmpiDrawing_API::SE_IID_GEOMETRYSINK_MPGUI, reinterpret_cast<void**>(geometrySink));
			}

			return hr == 0 ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
		}

		LinearGradientBrush_win7::LinearGradientBrush_win7(GmpiDrawing_API::IMpFactory* factory, ID2D1RenderTarget* context,
		                                                   const GmpiDrawing_API::MP1_LINEAR_GRADIENT_BRUSH_PROPERTIES*
		                                                   linearGradientBrushProperties,
		                                                   const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties,
		                                                   const GmpiDrawing_API::IMpGradientStopCollection*
		                                                   gradientStopCollection): Brush(nullptr, factory)
		{
			auto nativegsc = ((GradientStopCollection*)gradientStopCollection)->native();

			std::vector<GmpiDrawing_API::MP1_GRADIENT_STOP> stops;
			stops.assign(nativegsc->GetGradientStopCount(), GmpiDrawing_API::MP1_GRADIENT_STOP());
			nativegsc->GetGradientStops((D2D1_GRADIENT_STOP*) stops.data(), nativegsc->GetGradientStopCount());

			for( auto&s : stops )
			{
				s.color.r = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(s.color.r));
				s.color.g = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(s.color.g));
				s.color.b = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(s.color.b));
			}

			ID2D1GradientStopCollection* gradientStopsCorrected;
			context->CreateGradientStopCollection((D2D1_GRADIENT_STOP *) stops.data(), (UINT32) stops.size(), &gradientStopsCorrected);

			HRESULT hr = context->CreateLinearGradientBrush(
				(D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES*)linearGradientBrushProperties, (D2D1_BRUSH_PROPERTIES*)brushProperties,
				gradientStopsCorrected, (ID2D1LinearGradientBrush **)&native_);
			assert(hr == 0);

			gradientStopsCorrected->Release();
		}

		int32_t TextFormat::GetFontMetrics(GmpiDrawing_API::MP1_FONT_METRICS* returnFontMetrics)
		{
			IDWriteFontCollection *collection;
			IDWriteFontFamily *family;
			IDWriteFontFace *fontface;
			IDWriteFont *font;
			WCHAR nameW[255];
			UINT32 index;
			BOOL exists;
			HRESULT hr;

			hr = native()->GetFontCollection(&collection);
			//	ok(hr == S_OK, "got 0x%08x\n", hr);

			hr = native()->GetFontFamilyName(nameW, sizeof(nameW) / sizeof(WCHAR));
			//	ok(hr == S_OK, "got 0x%08x\n", hr);

			hr = collection->FindFamilyName(nameW, &index, &exists);
			if (exists == 0) // font not available. Fallback.
			{
				index = 0;
			}

			hr = collection->GetFontFamily(index, &family);
			//	ok(hr == S_OK, "got 0x%08x\n", hr);
			collection->Release();

			hr = family->GetFirstMatchingFont(
				native()->GetFontWeight(),
				native()->GetFontStretch(),
				native()->GetFontStyle(),
				&font);
			//	ok(hr == S_OK, "got 0x%08x\n", hr);

			hr = font->CreateFontFace(&fontface);
			//	ok(hr == S_OK, "got 0x%08x\n", hr);

			font->Release();
			family->Release();

			DWRITE_FONT_METRICS metrics;
			fontface->GetMetrics(&metrics);
			fontface->Release();

			// Sizes returned must always be in DIPs.
			float emsToDips = native()->GetFontSize() / metrics.designUnitsPerEm;

			returnFontMetrics->ascent = emsToDips * metrics.ascent;
			returnFontMetrics->descent = emsToDips * metrics.descent;
			returnFontMetrics->lineGap = emsToDips * metrics.lineGap;
			returnFontMetrics->capHeight = emsToDips * metrics.capHeight;
			returnFontMetrics->xHeight = emsToDips * metrics.xHeight;
			returnFontMetrics->underlinePosition = emsToDips * metrics.underlinePosition;
			returnFontMetrics->underlineThickness = emsToDips * metrics.underlineThickness;
			returnFontMetrics->strikethroughPosition = emsToDips * metrics.strikethroughPosition;
			returnFontMetrics->strikethroughThickness = emsToDips * metrics.strikethroughThickness;

			return gmpi::MP_OK;
		}

		void TextFormat::GetTextExtentU(const char* utf8String, int32_t stringLength, GmpiDrawing_API::MP1_SIZE* returnSize)
		{
			//	std::string cstring(utf8String, stringLength);
			auto widestring = stringConverter->from_bytes(utf8String, utf8String + stringLength);

			IDWriteFactory* writeFactory = 0;
			auto hr = DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(writeFactory),
				reinterpret_cast<IUnknown **>(&writeFactory)
			);

			IDWriteTextLayout* pTextLayout_ = 0;

			hr = writeFactory->CreateTextLayout(
				widestring.data(),      // The string to be laid out and formatted.
				(UINT32)widestring.size(),  // The length of the string.
				native(),  // The text format to apply to the string (contains font information, etc).
				100000,         // The width of the layout box.
				100000,        // The height of the layout box.
				&pTextLayout_  // The IDWriteTextLayout interface pointer.
			);

			DWRITE_TEXT_METRICS textMetrics;
			pTextLayout_->GetMetrics(&textMetrics);

			returnSize->height = textMetrics.height;
			returnSize->width = textMetrics.widthIncludingTrailingWhitespace;

			//auto th = pTextLayout_->GetMaxHeight(); // just return 100000
			//auto tw = pTextLayout_->GetMaxWidth();

			SafeRelease(pTextLayout_);
			SafeRelease(writeFactory);
		}

		/*
		//#undef DrawText
		//void GraphicsContext_DirectX::DrawTextU(const char* utf8String, int32_t stringLength, GmpiDrawing_API::IMpTextFormat* TextFormat, GmpiDrawing::Rect rect, GmpiDrawing_API::IMpBrush* brush, int32_t flags )
		void GraphicsContext_DirectX::DrawTextU(const char* utf8String, int32_t stringLength, const GmpiDrawing_API::IMpTextFormat* TextFormat, const GmpiDrawing_API::MP1_RECT* layoutRect, const GmpiDrawing_API::IMpBrush* brush, int32_t flags)
		{
			std::string cstring(utf8String, stringLength);
			auto widestring = stringConverter->from_bytes(cstring);

			auto b = ((Brush*)brush)->nativeBrush();
			auto tf = ((TextFormat*)TextFormat)->native();
			context_->DrawText(widestring.data(), (UINT32)widestring.size(), tf, (D2D1_RECT_F*)layoutRect, b, (D2D1_DRAW_TEXT_OPTIONS)flags);
		}
		*/

		// Create factory myself;
		Factory::Factory() :
			writeFactory(nullptr)
			, pIWICFactory(nullptr)
			, m_pDirect2dFactory(nullptr)
			, DX_support_sRGB(true)
		{
		}

		void Factory::Init(ID2D1Factory1* existingFactory)
		{
			if (existingFactory)
			{
				m_pDirect2dFactory = existingFactory;
				m_pDirect2dFactory->AddRef();
			}
			else
			{
				D2D1_FACTORY_OPTIONS o;
				o.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#ifdef _DEBUG
	//			o.debugLevel = D2D1_DEBUG_LEVEL_WARNING; // Need to install special stuff. https://msdn.microsoft.com/en-us/library/windows/desktop/ee794278%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396 
#endif
// wrong UUID	auto rs = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &o, (void**)&m_pDirect2dFactory);
				auto rs = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &o, (void**)&m_pDirect2dFactory);

				if (FAILED(rs))
				{
					_RPT1(_CRT_WARN, "D2D1CreateFactory FAIL %d\n", rs);
					return;  // Fail.
				}

				//		_RPT2(_CRT_WARN, "D2D1CreateFactory OK %d : %x\n", rs, m_pDirect2dFactory);
			}

			writeFactory = nullptr;

			auto hr = DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(writeFactory),
				reinterpret_cast<IUnknown**>(&writeFactory)
			);

			pIWICFactory = nullptr;

			hr = CoCreateInstance(
				CLSID_WICImagingFactory,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_IWICImagingFactory,
				(LPVOID*)&pIWICFactory
			);

			// Cache font family names
			{
				IDWriteFontCollection* fonts = nullptr;
				writeFactory->GetSystemFontCollection(&fonts);

				auto count = fonts->GetFontFamilyCount();

				for (int index = 0; index < (int)count; ++index)
				{
					IDWriteFontFamily* family = nullptr;
					fonts->GetFontFamily(index, &family);

					IDWriteLocalizedStrings* names = nullptr;
					family->GetFamilyNames(&names);

					BOOL exists;
					unsigned int nameIndex;
					names->FindLocaleName(L"en-us", &nameIndex, &exists);
					if (exists)
					{
						wchar_t name[64];
						names->GetString(nameIndex, name, sizeof(name) / sizeof(name[0]));
						//						_RPTW1(_CRT_WARN, L"%s\n", name);
						std::transform(name, name + wcslen(name), name, ::tolower);

						supportedFontFamilies.push_back(name);
					}

					names->Release();
					family->Release();
				}

				fonts->Release();
			}
		}

		Factory::~Factory()
		{
			SafeRelease(m_pDirect2dFactory);
			SafeRelease(writeFactory);
			SafeRelease(pIWICFactory);
		}

		int32_t Factory::CreatePathGeometry(GmpiDrawing_API::IMpPathGeometry** pathGeometry)
		{
			*pathGeometry = nullptr;
			//*pathGeometry = new GmpiGuiHosting::PathGeometry();
			//return gmpi::MP_OK;

			ID2D1PathGeometry* d2d_geometry = nullptr;
			HRESULT hr = m_pDirect2dFactory->CreatePathGeometry(&d2d_geometry);

			if (hr == 0)
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new gmpi::directx::Geometry(d2d_geometry));

				b2->queryInterface(GmpiDrawing_API::SE_IID_PATHGEOMETRY_MPGUI, reinterpret_cast<void**>(pathGeometry));
			}

			return hr == 0 ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
		}

		int32_t Factory::CreateTextFormat(const char* fontFamilyName, void* unused /* fontCollection */, GmpiDrawing_API::MP1_FONT_WEIGHT fontWeight, GmpiDrawing_API::MP1_FONT_STYLE fontStyle, GmpiDrawing_API::MP1_FONT_STRETCH fontStretch, float fontSize, void* unused2 /* localeName */, GmpiDrawing_API::IMpTextFormat** TextFormat)
		{
			*TextFormat = nullptr;

			auto fontFamilyNameW = stringConverter.from_bytes(fontFamilyName);
			std::wstring lowercaseName(fontFamilyNameW);
			std::transform(lowercaseName.begin(), lowercaseName.end(), lowercaseName.begin(), ::tolower);

			if (std::find(supportedFontFamilies.begin(), supportedFontFamilies.end(), lowercaseName) == supportedFontFamilies.end())
			{
				fontFamilyNameW = fontMatch(fontFamilyNameW, fontWeight, fontSize);
			}

			IDWriteTextFormat* dwTextFormat = nullptr;

			auto hr = writeFactory->CreateTextFormat(
				fontFamilyNameW.c_str(),
				NULL,
				(DWRITE_FONT_WEIGHT)fontWeight,
				(DWRITE_FONT_STYLE)fontStyle,
				(DWRITE_FONT_STRETCH)fontStretch,
				fontSize,
				L"", //locale
				&dwTextFormat
			);

			if (hr == 0)
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new gmpi::directx::TextFormat(&stringConverter, dwTextFormat));

				b2->queryInterface(GmpiDrawing_API::SE_IID_TEXTFORMAT_MPGUI, reinterpret_cast<void**>(TextFormat));
			}

			return hr == 0 ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
		}

		// 2nd pass - GDI->DirectWrite conversion. "Arial Black" -> "Arial"
		std::wstring Factory::fontMatch(std::wstring fontFamilyNameW, GmpiDrawing_API::MP1_FONT_WEIGHT fontWeight, float fontSize)
		{
			auto it = GdiFontConversions.find(fontFamilyNameW);
			if (it != GdiFontConversions.end())
			{
				return (*it).second;
			}

			IDWriteGdiInterop* interop = nullptr;
			writeFactory->GetGdiInterop(&interop);

			LOGFONT lf;
			memset(&lf, 0, sizeof(LOGFONT));   // Clear out structure.
			lf.lfHeight = (LONG) -fontSize;
			lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
			const wchar_t* actual_facename = fontFamilyNameW.c_str();

			if (fontFamilyNameW == _T("serif"))
			{
				actual_facename = _T("Times New Roman");
				lf.lfPitchAndFamily = DEFAULT_PITCH | FF_ROMAN;
			}

			if (fontFamilyNameW == _T("sans-serif"))
			{
				//		actual_facename = _T("Helvetica");
				actual_facename = _T("Arial"); // available on all version of windows
				lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
			}

			if (fontFamilyNameW == _T("cursive"))
			{
				actual_facename = _T("Zapf-Chancery");
				lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SCRIPT;
			}

			if (fontFamilyNameW == _T("fantasy"))
			{
				actual_facename = _T("Western");
				lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DECORATIVE;
			}

			if (fontFamilyNameW == _T("monospace"))
			{
				actual_facename = _T("Courier New");
				lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
			}
			wcscpy_s(lf.lfFaceName, 32, actual_facename);
			/*
			if ((p_desc->flags & TTL_UNDERLINE) != 0)
			{
			lf.lfUnderline = 1;
			}
			*/
			if (fontWeight > GmpiDrawing_API::MP1_FONT_WEIGHT_SEMI_BOLD)
			{
				lf.lfWeight = FW_BOLD;
			}
			else
			{
				if (fontWeight < 350)
				{
					lf.lfWeight = FW_LIGHT;
				}
			}

			IDWriteFont* font = nullptr;
			auto hr = interop->CreateFontFromLOGFONT(&lf, &font);

			if (font)
			{
				IDWriteFontFamily* family = nullptr;
				font->GetFontFamily(&family);

				IDWriteLocalizedStrings* names = nullptr;
				family->GetFamilyNames(&names);

				BOOL exists;
				unsigned int nameIndex;
				names->FindLocaleName(L"en-us", &nameIndex, &exists);
				if (exists)
				{
					wchar_t name[64];
					names->GetString(nameIndex, name, sizeof(name) / sizeof(name[0]));
					std::transform(name, name + wcslen(name), name, ::tolower);

					//						supportedFontFamilies.push_back(name);
					GdiFontConversions.insert(std::pair<std::wstring, std::wstring>(fontFamilyNameW, name));
					fontFamilyNameW = name;
				}

				names->Release();
				family->Release();

				font->Release();
			}

			interop->Release();
			return fontFamilyNameW;
		}

		int32_t Factory::CreateImage(int32_t width, int32_t height, GmpiDrawing_API::IMpBitmap** returnDiBitmap)
		{
			IWICBitmap* wicBitmap = nullptr;
			auto hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &wicBitmap); // pre-muliplied alpha
	// nuh	auto hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat32bppBGRA, WICBitmapCacheOnLoad, &wicBitmap);

			if (hr == 0)
			{
				gmpi_sdk::mp_shared_ptr<Bitmap> b2;
				b2.Attach(new Bitmap(this, wicBitmap));

				b2->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_MPGUI, (void**)returnDiBitmap);
			}

			SafeRelease(wicBitmap);

			return gmpi::MP_OK;
		}

		IWICBitmap* Factory::CreateDiBitmapFromNative(ID2D1Bitmap* D2D_Bitmap)
		{
			/*
			probly need to:
			cast bitmap to rendertarget,
			copy bits out with ID2D1Bitmap::CopyFromRenderTarget 
			create a new IWICBitmap
			create a WicBitmapRenderTarget
			create a 2nd bitmap FROM THE wic render target
			copy bits into new bitmap
			draw new bitmap on rendertarget.
			*/

			// Don't work, can't draw from bitmap belonging to different render target. !!!
			auto size = D2D_Bitmap->GetSize();

			// Create a WIC bitmap.
			IWICBitmap* wicBitmap = nullptr;
			auto hr = pIWICFactory->CreateBitmap((UINT) size.width, (UINT) size.height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &wicBitmap); // pre-muliplied alpha

			// Get a rendertarget on wic bitmap.
			D2D1_RENDER_TARGET_PROPERTIES props;
			memset(&props, 0, sizeof(props));
			props.type = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
			props.dpiX = 0.0f;
			props.dpiY = 0.0f;
			props.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM; // DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

			ID2D1RenderTarget* wicRenderTarget = nullptr;
			hr = m_pDirect2dFactory->CreateWicBitmapRenderTarget(
				wicBitmap,
				&props,
				&wicRenderTarget);

			// Draw pixels to wic bitmap.
			wicRenderTarget->BeginDraw();

			GmpiDrawing::Rect r(0,0, size.width, size.height);

			wicRenderTarget->DrawBitmap(D2D_Bitmap, (D2D1_RECT_F*)&r, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, (D2D1_RECT_F*)&r);
			
			ID2D1SolidColorBrush *solidColorBrush;
			D2D1_COLOR_F color;
			D2D1_RECT_F rect;
			rect.left = rect.top = 0;
			rect.right = rect.bottom = 10;
			color.r = color.g = color.b = color.a = 0.8f;
			wicRenderTarget->CreateSolidColorBrush(color, &solidColorBrush);
			wicRenderTarget->FillRectangle(rect, solidColorBrush);

			auto r2 = wicRenderTarget->EndDraw();

			// release render target.
			SafeRelease(wicRenderTarget);

			return wicBitmap;
		}

		int32_t Factory::LoadImageU(const char* utf8Uri, GmpiDrawing_API::IMpBitmap** returnDiBitmap)
		{
			*returnDiBitmap = nullptr;

			/*
			// Castr stream to protectedfile object.
			ProtectedFile2* pf = (ProtectedFile2*)stream;
			std::string uri = pf->getFullUri();

			// Initialize the in-memory stream where data will be stored.
			#if defined (SE_TARGET_WINDOWS_STORE_APP)
			auto winrtStream = ref new InMemoryRandomAccessStream();

			// TODO Write (async!!)

			ComPtr<IStream> pStream;
			//	DX::ThrowIfFailed(
			CreateStreamOverRandomAccessStream(winrtStream, IID_PPV_ARGS(&pStream));
			//		);
			IStream* lStream = pStream.Get();
			#else

			// allocate buffer and create stream. Inefficiant as creates extra copy in memory.
			IStream *lStream = NULL;
			int64_t size;
			pf->getSize(&size);
			HGLOBAL m_hBuffer = ::GlobalAlloc(GMEM_FIXED, (SIZE_T)size);

			// fill the buffer with image data
			pf->read((char*)m_hBuffer, size);

			::CreateStreamOnHGlobal(m_hBuffer, TRUE, &lStream); // TRUE indicates pStream->Release() should free HGLOBAL automatically.
			#endif

			IWICBitmapDecoder* pDecoder = NULL;
			if (hr == 0)
			{
			hr = pIWICFactory->CreateDecoderFromStream(
			lStream,
			NULL,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
			);
			}
			*/

			//	std::string cstring(utf8Uri);
			auto uriW = stringConverter.from_bytes(utf8Uri);

			IWICBitmapDecoder* pDecoder = NULL;
			// To load a bitmap from a file, first use WIC objects to load the image and to convert it to a Direct2D-compatible format.
			auto hr = pIWICFactory->CreateDecoderFromFilename(
				uriW.c_str(),
				NULL,
				GENERIC_READ,
				WICDecodeMetadataCacheOnLoad,
				&pDecoder
			);

			/*
		#if !defined (SE_TARGET_WINDOWS_STORE_APP)
		pStream->Release(); // may not release HGLOBAL because GDI plus maintains a reference or 2.
		#endif
		*/
			IWICBitmapFrameDecode *pSource = NULL;
			if (hr == 0)
			{
				// 2.Retrieve a frame from the image and store the frame in an IWICBitmapFrameDecode object.
				hr = pDecoder->GetFrame(0, &pSource);
			}

			IWICFormatConverter *pConverter = NULL;
			if (hr == 0)
			{
				// 3.The bitmap must be converted to a format that Direct2D can use.
				hr = pIWICFactory->CreateFormatConverter(&pConverter);
			}
			if (hr == 0)
			{
				//WICPixelFormatGUID Guid;
				//pSource->GetPixelFormat(&Guid);

				hr = pConverter->Initialize(
					pSource,
					GUID_WICPixelFormat32bppPBGRA, //Premultiplied
					WICBitmapDitherTypeNone,
					NULL,
					0.f,
					WICBitmapPaletteTypeCustom
				);
			}

			IWICBitmap* wicBitmap = nullptr;
			if (hr == 0)
			{
				hr = pIWICFactory->CreateBitmapFromSource(
					pConverter,
					WICBitmapCacheOnLoad,
					&wicBitmap);
			}
			/*
D3D11 ERROR: ID3D11Device::CreateTexture2D: The Dimensions are invalid. For feature level D3D_FEATURE_LEVEL_11_0, the Width (value = 32) must be between 1 and 16384, inclusively. The Height (value = 60000) must be between 1 and 16384, inclusively. And, the ArraySize (value = 1) must be between 1 and 2048, inclusively. [ STATE_CREATION ERROR #101: CREATETEXTURE2D_INVALIDDIMENSIONS]
			*/
			if (hr == 0)
			{
				UINT width, height;

				wicBitmap->GetSize(&width, &height);

				const int maxDirectXImageSize = 16384;
				if (width > maxDirectXImageSize || height > maxDirectXImageSize)
				{
					hr = -1; // fail, too big for DirectX.
#if !defined( SE_EDIT_SUPPORT2 ) && defined( SE_SUPPORT_MFC )
					std::wostringstream oss;
					oss << L"Sorry, Image too large:" << uriW << L"\nMaximum dimensions is 16384 pixels.\n";
					MessageBox( 0,oss.str().c_str(), L"", MB_OK | MB_ICONSTOP);
					MessageBox(NULL, (LPCTSTR)oss.str().c_str(), _T("Error"), MB_OK | MB_ICONINFORMATION);
#endif
				}
				else
				{
					auto bitmap = new Bitmap(this, wicBitmap);
#ifdef _DEBUG
					bitmap->debugFilename = utf8Uri;
#endif
					gmpi_sdk::mp_shared_ptr<GmpiDrawing_API::IMpBitmap> b2;
					b2.Attach(bitmap);
					b2->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_MPGUI, (void**)returnDiBitmap);
				}
			}

			SafeRelease(pDecoder);
			SafeRelease(pSource);
			SafeRelease(pConverter);
			SafeRelease(wicBitmap);

			return hr == 0 ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
		}

		void GraphicsContext::DrawGeometry(const GmpiDrawing_API::IMpPathGeometry* geometry, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle)
		{
			auto& d2d_geometry = ((gmpi::directx::Geometry*)geometry)->geometry_;
			context_->DrawGeometry(d2d_geometry, ((Brush*)brush)->nativeBrush(), (FLOAT)strokeWidth, toNative(strokeStyle));
		}

		void GraphicsContext::DrawTextU(const char* utf8String, int32_t stringLength, const GmpiDrawing_API::IMpTextFormat* textFormat, const GmpiDrawing_API::MP1_RECT* layoutRect, const GmpiDrawing_API::IMpBrush* brush, int32_t flags)
		{
#if 0 // compare speed of two conversion functions.
			std::string testString("the quick brown fox jumps over the lazy dog.");

			auto start = std::chrono::steady_clock::now();
			std::wstring test;
			for (int i = 0; i < 1000; ++i)
			{
				test = stringConverter->from_bytes(testString);
			}

			auto duration = std::chrono::steady_clock::now() - start;
			auto us1 = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

			start = std::chrono::steady_clock::now();
			for (int i = 0; i < 1000; ++i)
			{
				test = FastUnicode::Utf8ToWstring(testString.c_str());
			}
			duration = std::chrono::steady_clock::now() - start;
			auto us2 = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

			_RPT2(_CRT_WARN, "%d %d\n", (int)us1, (int)us2);

			wchar_t output[100];
			swprintf(output, L"%d %d", (int)us1, (int)us2);
			std::wstring widestring(output);
#endif
			auto widestring = stringConverter->from_bytes(utf8String, utf8String + stringLength);

			auto b = ((Brush*)brush)->nativeBrush();
			auto tf = ((TextFormat*)textFormat)->native();

			context_->DrawText(widestring.data(), (UINT32)widestring.size(), tf, reinterpret_cast<const D2D1_RECT_F*>(layoutRect), b, (D2D1_DRAW_TEXT_OPTIONS)flags);
		}

		void Bitmap::GetFactory(GmpiDrawing_API::IMpFactory** pfactory)
		{
			*pfactory = factory;
		}

		int32_t Bitmap::lockPixels(GmpiDrawing_API::IMpBitmapPixels** returnInterface, int32_t flags)
		{
			*returnInterface = nullptr;


			// If image was not loaded from a WicBitmap (i.e. was created from device context), then need to write it to WICBitmap first.
			if (diBitmap_ == nullptr)
			{
				return gmpi::MP_FAIL; // creating WIC from D2DBitmap not implemented.

				assert(nativeBitmap_);
				diBitmap_ = factory->CreateDiBitmapFromNative(nativeBitmap_);
			}

			gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
			b2.Attach(new bitmapPixels(nativeBitmap_, diBitmap_, true, flags));

			return b2->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_PIXELS_MPGUI, (void**)(returnInterface));
		}

		ID2D1Bitmap* Bitmap::GetNativeBitmap(GraphicsContext* graphics)
		{
			// Check for loss of surface.
			if (graphics->native() != nativeContext_ && diBitmap_ != nullptr)
			{
				if (nativeBitmap_)
				{
					nativeBitmap_->Release();
					nativeBitmap_ = nullptr;
				}

				nativeContext_ = graphics->native();

				{
					auto maxSize = nativeContext_->GetMaximumBitmapSize();
					UINT imageW, imageH;
					diBitmap_->GetSize(&imageW, &imageH);

					if (imageW > maxSize || imageH > maxSize)
					{
						assert(false); // IMAGE TOO BIG!
						return nullptr;
					}
				}

				D2D1_BITMAP_PROPERTIES props;
				props.dpiX = props.dpiY = 96;
				props.pixelFormat.format = nativeContext_->GetPixelFormat().format;
				props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

				// Convert to D2D format and cache.
				auto hr = nativeContext_->CreateBitmapFromWicBitmap(
					diBitmap_,
					&props, //NULL,
					&nativeBitmap_
				);

				if (!graphics->SupportSRGB())
				{
					ApplyAlphaCorrection_win7();
				}
			}

			return nativeBitmap_;
		}

		// WIX premultiplies images automatically on load, but wrong (assumes linear not SRGB space). Fix it.
		void Bitmap::ApplyPreMultiplyCorrection()
		{
#if 1
			GmpiDrawing::Bitmap bitmap(this);

			auto pixelsSource = bitmap.lockPixels(true);
			auto imageSize = bitmap.GetSize();
			int totalPixels = (int)imageSize.height * pixelsSource.getBytesPerRow() / sizeof(uint32_t);
			uint8_t* sourcePixels = pixelsSource.getAddress();

			// WIX currently not premultiplying correctly, so redo it respecing gamma.
			const double over255 = 1.0 / 255.0;
			for (int i = 0; i < totalPixels; ++i)
			{
				int alpha = sourcePixels[3];

				if (alpha != 255 && alpha != 0)
				{
					float AlphaNorm = alpha / 255.f;
					float overAlphaNorm = 1.f / AlphaNorm;

					for (int j = 0; j < 3; ++j)
					{
						int p = sourcePixels[j];
						if (p != 0)
						{
							float originalPixel = p * overAlphaNorm; // un-premultiply.

							// To linear
							auto cf = se_sdk::FastGamma::sRGB_to_float(FastRealToIntTruncateTowardZero(originalPixel + 0.5f));

							cf *= AlphaNorm;						// pre-multiply (correctly).

							// back to SRGB
							sourcePixels[j] = se_sdk::FastGamma::float_to_sRGB(cf);
						}
					}
				}

				sourcePixels += sizeof(uint32_t);
			}
#if 0
			for (unsigned char i = 0; i < 256; ++i)
			{
				_RPT2(_CRT_WARN, "%f %f\n", FastGamma::sRGB_to_float(i), (float)FastGamma::float_to_sRGB(FastGamma::sRGB_to_float(i)));
			}
#endif
#endif
		}

		void Bitmap::ApplyAlphaCorrection_win7()
		{

#if 1 // apply gamma correction to compensate for linear blendinging in SRGB space (DirectX 1.0 limitation)
			GmpiDrawing::Bitmap bitmap(this);

			auto pixelsSource = bitmap.lockPixels(true);
			auto imageSize = bitmap.GetSize();
			int totalPixels = (int)imageSize.height * pixelsSource.getBytesPerRow() / sizeof(uint32_t);

			uint8_t* sourcePixels = pixelsSource.getAddress();
			const float gamma = 2.2f;
			const float overTwoFiftyFive = 1.0f / 255.0f;
			for (int i = 0; i < totalPixels; ++i)
			{
				int alpha = sourcePixels[3];

				if (alpha != 0 && alpha != 255)
				{
					float bitmapAlpha = alpha * overTwoFiftyFive;

					// Calc pixel lumination (linear).
					float components[3];
					float foreground = 0.0f;
					for (int c = 0; c < 3; ++c)
					{
						float pixel = sourcePixels[c] * overTwoFiftyFive;
						pixel /= bitmapAlpha; // un-premultiply
						pixel = powf(pixel, gamma);
						components[c] = pixel;
					}
					//					foreground = 0.2126 * components[2] + 0.7152 * components[1] + 0.0722 * components[0]; // Luminance.
					foreground = 0.3333f * components[2] + 0.3333f * components[1] + 0.3333f * components[0]; // Average. Much the same as Luminance.

					float blackAlpha = 1.0f - powf(1.0f - bitmapAlpha, 1.0 / gamma);
					float whiteAlpha = powf(bitmapAlpha, 1.0f / gamma);

					float mix = powf(foreground, 1.0f / gamma);

					float bitmapAlphaCorrected = blackAlpha * (1.0f - mix) + whiteAlpha * mix;

					for (int c = 0; c < 3; ++c)
					{
						float pixel = components[c];
						pixel = powf(pixel, 1.0f / gamma); // linear -> sRGB space.
						pixel *= bitmapAlphaCorrected; // premultiply
						pixel = pixel * 255.0f + 0.5f; // back to 8-bit
						sourcePixels[c] = (std::min)(255, FastRealToIntTruncateTowardZero(pixel));
					}

					bitmapAlphaCorrected = bitmapAlphaCorrected * 255.0f + 0.5f; // back to 8-bit
		//			int alphaVal = (int)(bitmapAlphaCorrected * 255.0f + 0.5f);
					sourcePixels[3] = FastRealToIntTruncateTowardZero(bitmapAlphaCorrected);
				}
				sourcePixels += sizeof(uint32_t);
			}
#endif
		}

		int32_t GraphicsContext::CreateSolidColorBrush(const GmpiDrawing_API::MP1_COLOR* color, GmpiDrawing_API::IMpSolidColorBrush **solidColorBrush)
		{
			*solidColorBrush = nullptr;

			ID2D1SolidColorBrush* b = nullptr;
			HRESULT hr = context_->CreateSolidColorBrush(*(D2D1_COLOR_F*)color, &b);

			if (hr == 0)
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new SolidColorBrush(b, factory));

				b2->queryInterface(GmpiDrawing_API::SE_IID_SOLIDCOLORBRUSH_MPGUI, reinterpret_cast<void **>(solidColorBrush));
			}

			return hr == 0 ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
		}

		int32_t GraphicsContext::CreateGradientStopCollection(const GmpiDrawing_API::MP1_GRADIENT_STOP *gradientStops, uint32_t gradientStopsCount, GmpiDrawing_API::IMpGradientStopCollection** gradientStopCollection)
		{
			*gradientStopCollection = nullptr;

			ID2D1GradientStopCollection* native1 = nullptr;

			HRESULT hr = 0;

			if (context2_)
			{
				// New way. Gradients without banding.
				// requires ID2D1DeviceContext, not merely ID2D1RenderTarget
				// ID2D1GradientStopCollection1* native2 = nullptr;
#if 1
				hr = context2_->CreateGradientStopCollection(
					(D2D1_GRADIENT_STOP*)gradientStops,
					gradientStopsCount,
					D2D1_GAMMA_2_2,	// gamma-correct, but not smooth.
					//	D2D1_GAMMA_1_0, // smooth, but not gamma-correct.
					D2D1_EXTEND_MODE_CLAMP,
					&native1);
 
#else
				ID2D1GradientStopCollection1* native = nullptr;

				hr = context2_->CreateGradientStopCollection(
					(D2D1_GRADIENT_STOP*)gradientStops,
					gradientStopsCount,
					D2D1_COLOR_SPACE_SRGB,
					D2D1_COLOR_SPACE_SRGB,
					D2D1_BUFFER_PRECISION_UNKNOWN,
					D2D1_EXTEND_MODE_CLAMP,
					D2D1_COLOR_INTERPOLATION_MODE_STRAIGHT,
					&native
				);

				// hr returns 0x8899000a : A call to this method is invalid.

				native1 = (ID2D1GradientStopCollection*) native;
#endif
			}
			else
			{
				// for proper gradient in SRGB target, need to set gamma. hmm not sure. https://msdn.microsoft.com/en-us/library/windows/desktop/dd368113(v=vs.85).aspx
				hr = context_->CreateGradientStopCollection(
					(D2D1_GRADIENT_STOP*)gradientStops,
					gradientStopsCount,
					D2D1_GAMMA_2_2,	// gamma-correct, but not smooth.
					//	D2D1_GAMMA_1_0, // smooth, but not gamma-correct.
					D2D1_EXTEND_MODE_CLAMP,
					&native1);
			}

			if (hr == 0)
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> wrapper;
				wrapper.Attach(new GradientStopCollection(native1, factory));

				wrapper->queryInterface(GmpiDrawing_API::SE_IID_GRADIENTSTOPCOLLECTION_MPGUI, reinterpret_cast<void**>(gradientStopCollection));
			}

			return hr == 0 ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
		}


		//int32_t GraphicsContext::CreateMesh(GmpiDrawing_API::IMpMesh** returnObject)
		//{
		//	*returnObject = nullptr;

		//	auto mesh = new Mesh(factory, context_);
		//	return mesh->queryInterface(GmpiDrawing_API::SE_IID_MESH_MPGUI, reinterpret_cast<void **>(returnObject));
		//}
		/*
		int32_t GraphicsContext::CreateBitmap(GmpiDrawing_API::MP1_SIZE_U size, const GmpiDrawing_API::MP1_BITMAP_PROPERTIES* bitmapProperties, GmpiDrawing_API::IMpBitmap** bitmap)
		{
			*bitmap = nullptr;

			D2D1_BITMAP_PROPERTIES nativeBitmapProperties;
			nativeBitmapProperties.dpiX = 0.0f;
			nativeBitmapProperties.dpiY = 0.0f;
			nativeBitmapProperties.pixelFormat = context_->GetPixelFormat();

			ID2D1Bitmap* b = nullptr;
			auto hr = context_->CreateBitmap(*(D2D1_SIZE_U*) &size, nativeBitmapProperties, &b);

			if (hr == 0)
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new  bitmap(context_, b));

				b2->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_MPGUI, reinterpret_cast<void **>(bitmap));
			}

			return hr == 0 ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
		}
		*/

		int32_t GraphicsContext::CreateCompatibleRenderTarget(const GmpiDrawing_API::MP1_SIZE* desiredSize, GmpiDrawing_API::IMpBitmapRenderTarget** returnObject)
		{
			*returnObject = nullptr;

			gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
			b2.Attach(new BitmapRenderTarget(this, desiredSize, factory));
			return b2->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_RENDERTARGET_MPGUI, reinterpret_cast<void **>(returnObject));
		}

		int32_t BitmapRenderTarget::GetBitmap(GmpiDrawing_API::IMpBitmap** returnBitmap)
		{
			*returnBitmap = nullptr;

			ID2D1Bitmap* bitmap;
			auto hr = ((ID2D1BitmapRenderTarget*)context_)->GetBitmap(&bitmap);

			if (hr == 0)
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new Bitmap(factory, context_, bitmap));
				bitmap->Release();

				b2->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_MPGUI, reinterpret_cast<void **>(returnBitmap));
			}

			return hr == 0 ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
		}

		void GraphicsContext::PushAxisAlignedClip(const GmpiDrawing_API::MP1_RECT* clipRect/*, GmpiDrawing_API::MP1_ANTIALIAS_MODE antialiasMode*/)
		{
			context_->PushAxisAlignedClip((D2D1_RECT_F*)clipRect, D2D1_ANTIALIAS_MODE_ALIASED /*, (D2D1_ANTIALIAS_MODE)antialiasMode*/);

			// Transform to original position.
			GmpiDrawing::Matrix3x2 currentTransform;
			context_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(&currentTransform));
			auto r2 = currentTransform.TransformRect(*clipRect);
			clipRectStack.push_back(r2);

//			_RPT4(_CRT_WARN, "                 PushAxisAlignedClip( %f %f %f %f)\n", r2.left, r2.top, r2.right , r2.bottom);
		}

		void GraphicsContext::GetAxisAlignedClip(GmpiDrawing_API::MP1_RECT* returnClipRect)
		{
			// Transform to original position.
			GmpiDrawing::Matrix3x2 currentTransform;
			context_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(&currentTransform));
			currentTransform.Invert();
			auto r2 = currentTransform.TransformRect(clipRectStack.back());

			*returnClipRect = r2;
		}

	} // namespace
} // namespace