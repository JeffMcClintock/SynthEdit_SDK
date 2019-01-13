#pragma once

/*
#include "DirectXGfx.h"
*/

#if defined( SE_SUPPORT_MFC )
#include <afxwin.h>         // MFC core and standard components
#endif

#include <d2d1_2.h>
#include <dwrite.h>
#include <codecvt>
#include <Wincodec.h>
#include "./gmpi_gui_hosting.h"

namespace gmpi
{
	namespace directx
	{
		inline void SafeRelease(IUnknown* object)
		{
			if (object)
				object->Release();
		}

		// Classes without GetFactory()
		template<class MpInterface, class DxType>
		class GmpiDXWrapper : public MpInterface
		{
		protected:
			DxType* native_;

			~GmpiDXWrapper()
			{
				if (native_)
				{
					native_->Release();
				}
			}

		public:
			GmpiDXWrapper(DxType* native = nullptr) : native_(native) {}

			inline DxType* native()
			{
				return native_;
			}

			GMPI_REFCOUNT;
		};

		// Classes with GetFactory()
		template<class MpInterface, class DxType>
		class GmpiDXResourceWrapper : public GmpiDXWrapper<MpInterface, DxType>
		{
		protected:
			GmpiDrawing_API::IMpFactory* factory_;

		public:
			GmpiDXResourceWrapper(DxType* native, GmpiDrawing_API::IMpFactory* factory) : GmpiDXWrapper(native), factory_(factory) {}
			GmpiDXResourceWrapper(GmpiDrawing_API::IMpFactory* factory) : factory_(factory) {}

			virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory **factory) override
			{
				*factory = factory_;
			}

			GMPI_REFCOUNT;
		};

		/*
		class Resource //: public GmpiDrawing_API::IMpResource
		{
		protected:
			ID2D1Resource* native_;
			GmpiDrawing_API::IMpFactory* factory_;

			~Resource()
			{
				if (native_)
				{
					native_->Release();
				}
			}

		public:
			Resource(ID2D1Resource* native, GmpiDrawing_API::IMpFactory* factory) : native_(native), factory_(factory){}

			//inline ID2D1Resource* nativeResource()
			//{
			//	return (ID2D1Resource*)native_;
			//}
		};
		*/

		class Brush : /* public GmpiDrawing_API::IMpBrush,*/ public GmpiDXResourceWrapper<GmpiDrawing_API::IMpBrush, ID2D1Brush> // Resource
		{
		public:
			Brush(ID2D1Brush* native, GmpiDrawing_API::IMpFactory* factory) : GmpiDXResourceWrapper(native, factory) {}

			inline ID2D1Brush* nativeBrush()
			{
				return (ID2D1Brush*)native_;
			}
			/*
				virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory **factory) override
				{
					*factory = factory_;
				}
			*/
		};

		class SolidColorBrush : /* Simulated: public GmpiDrawing_API::IMpSolidColorBrush,*/ public Brush
		{
		public:
			SolidColorBrush(ID2D1SolidColorBrush* b, GmpiDrawing_API::IMpFactory *factory) : Brush(b, factory) {}

			inline ID2D1SolidColorBrush* nativeSolidColorBrush()
			{
				return (ID2D1SolidColorBrush*)native_;
			}

			// IMPORTANT: Virtual functions much 100% match GmpiDrawing_API::IMpSolidColorBrush to simulate inheritance.
			virtual void MP_STDCALL SetColor(const GmpiDrawing_API::MP1_COLOR* color) // simulated: override
			{
//				D2D1::ConvertColorSpace(D2D1::ColorF*) color);
				nativeSolidColorBrush()->SetColor((D2D1::ColorF*) color);
			}
			virtual GmpiDrawing_API::MP1_COLOR MP_STDCALL GetColor() // simulated:  override
			{
				auto b = nativeSolidColorBrush()->GetColor();
				//		return GmpiDrawing::Color(b.r, b.g, b.b, b.a);
				GmpiDrawing_API::MP1_COLOR c;
				c.a = b.a;
				c.r = b.r;
				c.g = b.g;
				c.b = b.b;
				return c;
			}

			//	GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_SOLIDCOLORBRUSH_MPGUI, GmpiDrawing_API::IMpSolidColorBrush);

			virtual int32_t MP_STDCALL queryInterface(const gmpi::MpGuid& iid, void** returnInterface) override
			{
				*returnInterface = 0;
				if (iid == GmpiDrawing_API::SE_IID_SOLIDCOLORBRUSH_MPGUI || iid == gmpi::MP_IID_UNKNOWN)
				{
					// non-standard. Forcing this class (which has the correct vtable) to pretend it's the emulated interface.
					*returnInterface = reinterpret_cast<GmpiDrawing_API::IMpSolidColorBrush*>(this);
					addRef();
					return gmpi::MP_OK;
				}
				return gmpi::MP_NOSUPPORT;
			}

			GMPI_REFCOUNT;
		};

		class SolidColorBrush_Win7 : /* Simulated: public GmpiDrawing_API::IMpSolidColorBrush,*/ public Brush
		{
		public:
			SolidColorBrush_Win7(ID2D1RenderTarget* context, const GmpiDrawing_API::MP1_COLOR* color, GmpiDrawing_API::IMpFactory* factory) : Brush(nullptr, factory)
			{
				GmpiDrawing::Color modified;
				modified.a = color->a;
				modified.r = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color->r));
				modified.g = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color->g));
				modified.b = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color->b));
//				modified = GmpiDrawing::Color::Orange;

				/*HRESULT hr =*/ context->CreateSolidColorBrush(*(D2D1_COLOR_F*)&modified, (ID2D1SolidColorBrush**) &native_);
			}

			inline ID2D1SolidColorBrush* nativeSolidColorBrush()
			{
				return (ID2D1SolidColorBrush*)native_;
			}

			// IMPORTANT: Virtual functions much 100% match GmpiDrawing_API::IMpSolidColorBrush to simulate inheritance.
			virtual void MP_STDCALL SetColor(const GmpiDrawing_API::MP1_COLOR* color) // simulated: override
			{
				//				D2D1::ConvertColorSpace(D2D1::ColorF*) color);
				GmpiDrawing::Color modified;
				modified.a = color->a;
				modified.r = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color->r));
				modified.g = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color->g));
				modified.b = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color->b));
				nativeSolidColorBrush()->SetColor((D2D1::ColorF*) &modified);
			}

			virtual GmpiDrawing_API::MP1_COLOR MP_STDCALL GetColor() // simulated:  override
			{
				auto b = nativeSolidColorBrush()->GetColor();
				//		return GmpiDrawing::Color(b.r, b.g, b.b, b.a);
				GmpiDrawing_API::MP1_COLOR c;
				c.a = b.a;
				c.r = b.r;
				c.g = b.g;
				c.b = b.b;
				return c;
			}

			//	GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_SOLIDCOLORBRUSH_MPGUI, GmpiDrawing_API::IMpSolidColorBrush);

			virtual int32_t MP_STDCALL queryInterface(const gmpi::MpGuid& iid, void** returnInterface) override
			{
				*returnInterface = 0;
				if (iid == GmpiDrawing_API::SE_IID_SOLIDCOLORBRUSH_MPGUI || iid == gmpi::MP_IID_UNKNOWN)
				{
					// non-standard. Forcing this class (which has the correct vtable) to pretend it's the emulated interface.
					*returnInterface = reinterpret_cast<GmpiDrawing_API::IMpSolidColorBrush*>(this);
					addRef();
					return gmpi::MP_OK;
				}
				return gmpi::MP_NOSUPPORT;
			}

			GMPI_REFCOUNT;
		};

		class GradientStopCollection : public GmpiDXResourceWrapper<GmpiDrawing_API::IMpGradientStopCollection, ID2D1GradientStopCollection>
		{
		public:
			GradientStopCollection(ID2D1GradientStopCollection* native, GmpiDrawing_API::IMpFactory* factory) : GmpiDXResourceWrapper(native, factory) {}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_GRADIENTSTOPCOLLECTION_MPGUI, GmpiDrawing_API::IMpGradientStopCollection);
		};

		class LinearGradientBrush : /* Simulated: public GmpiDrawing_API::IMpLinearGradientBrush,*/ public Brush
		{
		public:
			LinearGradientBrush(GmpiDrawing_API::IMpFactory *factory, ID2D1RenderTarget* context, const GmpiDrawing_API::MP1_LINEAR_GRADIENT_BRUSH_PROPERTIES* linearGradientBrushProperties, const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties, const  GmpiDrawing_API::IMpGradientStopCollection* gradientStopCollection)
			 : Brush(nullptr, factory)
			{
				HRESULT hr = context->CreateLinearGradientBrush((D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES*)linearGradientBrushProperties, (D2D1_BRUSH_PROPERTIES*)brushProperties, ((GradientStopCollection*)gradientStopCollection)->native(), (ID2D1LinearGradientBrush **)&native_);
				assert(hr == 0);
			}

			inline ID2D1LinearGradientBrush* native()
			{
				return (ID2D1LinearGradientBrush*)native_;
			}

			// IMPORTANT: Virtual functions much 100% match simulated interface.
			virtual void MP_STDCALL SetStartPoint(GmpiDrawing_API::MP1_POINT startPoint) // simulated: override
			{
				native()->SetStartPoint(*reinterpret_cast<D2D1_POINT_2F*>(&startPoint));
			}
			virtual void MP_STDCALL SetEndPoint(GmpiDrawing_API::MP1_POINT endPoint) // simulated: override
			{
				native()->SetEndPoint(*reinterpret_cast<D2D1_POINT_2F*>(&endPoint));
			}

			//	GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_LINEARGRADIENTBRUSH_MPGUI, GmpiDrawing_API::IMpLinearGradientBrush);
			virtual int32_t MP_STDCALL queryInterface(const gmpi::MpGuid& iid, void** returnInterface) override
			{
				*returnInterface = 0;
				if (iid == GmpiDrawing_API::SE_IID_LINEARGRADIENTBRUSH_MPGUI || iid == gmpi::MP_IID_UNKNOWN)
				{
					// non-standard. Forcing this class (which has the correct vtable) to pretend it's the emulated interface.
					*returnInterface = reinterpret_cast<GmpiDrawing_API::IMpLinearGradientBrush*>(this);
					addRef();
					return gmpi::MP_OK;
				}
				return gmpi::MP_NOSUPPORT;
			}

			GMPI_REFCOUNT;
		};

		class LinearGradientBrush_win7 : /* Simulated: public GmpiDrawing_API::IMpLinearGradientBrush,*/ public Brush
		{
		public:
			LinearGradientBrush_win7(GmpiDrawing_API::IMpFactory* factory, ID2D1RenderTarget* context,
			                         const GmpiDrawing_API::MP1_LINEAR_GRADIENT_BRUSH_PROPERTIES* linearGradientBrushProperties,
			                         const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties,
			                         const GmpiDrawing_API::IMpGradientStopCollection* gradientStopCollection);

			inline ID2D1LinearGradientBrush* native()
			{
				return (ID2D1LinearGradientBrush*)native_;
			}

			// IMPORTANT: Virtual functions much 100% match simulated interface.
			virtual void MP_STDCALL SetStartPoint(GmpiDrawing_API::MP1_POINT startPoint) // simulated: override
			{
				native()->SetStartPoint(*reinterpret_cast<D2D1_POINT_2F*>(&startPoint));
			}
			virtual void MP_STDCALL SetEndPoint(GmpiDrawing_API::MP1_POINT endPoint) // simulated: override
			{
				native()->SetEndPoint(*reinterpret_cast<D2D1_POINT_2F*>(&endPoint));
			}

			//	GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_LINEARGRADIENTBRUSH_MPGUI, GmpiDrawing_API::IMpLinearGradientBrush);
			virtual int32_t MP_STDCALL queryInterface(const gmpi::MpGuid& iid, void** returnInterface) override
			{
				*returnInterface = 0;
				if (iid == GmpiDrawing_API::SE_IID_LINEARGRADIENTBRUSH_MPGUI || iid == gmpi::MP_IID_UNKNOWN)
				{
					// non-standard. Forcing this class (which has the correct vtable) to pretend it's the emulated interface.
					*returnInterface = reinterpret_cast<GmpiDrawing_API::IMpLinearGradientBrush*>(this);
					addRef();
					return gmpi::MP_OK;
				}
				return gmpi::MP_NOSUPPORT;
			}

			GMPI_REFCOUNT;
		};

		class StrokeStyle : public GmpiDXResourceWrapper<GmpiDrawing_API::IMpStrokeStyle, ID2D1StrokeStyle>
		{
		public:
			StrokeStyle(ID2D1StrokeStyle* native, GmpiDrawing_API::IMpFactory* factory) : GmpiDXResourceWrapper(native, factory) {}

			virtual GmpiDrawing_API::MP1_CAP_STYLE MP_STDCALL GetStartCap() override
			{
				return (GmpiDrawing_API::MP1_CAP_STYLE) native()->GetStartCap();
			}

			virtual GmpiDrawing_API::MP1_CAP_STYLE MP_STDCALL GetEndCap() override
			{
				return (GmpiDrawing_API::MP1_CAP_STYLE) native()->GetEndCap();
			}

			virtual GmpiDrawing_API::MP1_CAP_STYLE MP_STDCALL GetDashCap() override
			{
				return (GmpiDrawing_API::MP1_CAP_STYLE) native()->GetDashCap();
			}

			virtual float MP_STDCALL GetMiterLimit() override
			{
				return native()->GetMiterLimit();
			}

			virtual GmpiDrawing_API::MP1_LINE_JOIN MP_STDCALL GetLineJoin() override
			{
				return (GmpiDrawing_API::MP1_LINE_JOIN) native()->GetLineJoin();
			}

			virtual float MP_STDCALL GetDashOffset() override
			{
				return native()->GetDashOffset();
			}

			virtual GmpiDrawing_API::MP1_DASH_STYLE MP_STDCALL GetDashStyle() override
			{
				return (GmpiDrawing_API::MP1_DASH_STYLE) native()->GetDashStyle();
			}

			virtual uint32_t MP_STDCALL GetDashesCount() override
			{
				return native()->GetDashesCount();
			}

			virtual void MP_STDCALL GetDashes(float* dashes, uint32_t dashesCount) override
			{
				return native()->GetDashes(dashes, dashesCount);
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_STROKESTYLE_MPGUI, GmpiDrawing_API::IMpStrokeStyle);
		};

		inline ID2D1StrokeStyle* toNative(const GmpiDrawing_API::IMpStrokeStyle* strokeStyle)
		{
			if (strokeStyle)
			{
				return ((StrokeStyle*)strokeStyle)->native();
			}
			return nullptr;
		}

		class TessellationSink : public GmpiDXWrapper<GmpiDrawing_API::IMpTessellationSink, ID2D1TessellationSink>
		{
		public:
			TessellationSink(ID2D1Mesh* mesh)
			{
				HRESULT hr = mesh->Open(&native_);
				assert(hr == S_OK);
			}

			void MP_STDCALL AddTriangles(const GmpiDrawing_API::MP1_TRIANGLE* triangles, uint32_t trianglesCount) override
			{
				native_->AddTriangles((const D2D1_TRIANGLE*) triangles, trianglesCount);
			}

			int32_t MP_STDCALL Close() override
			{
				native_->Close();
				return MP_OK;
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_TESSELLATIONSINK_MPGUI, GmpiDrawing_API::IMpTessellationSink);
			GMPI_REFCOUNT;
		};
		
		class Mesh : public GmpiDXResourceWrapper<GmpiDrawing_API::IMpMesh, ID2D1Mesh>
		{
		public:
			Mesh(GmpiDrawing_API::IMpFactory* factory, ID2D1RenderTarget* context) :
				GmpiDXResourceWrapper(factory)
			{
				HRESULT hr = context->CreateMesh(&native_);
				assert(hr == S_OK);
			}

			// IMpMesh
			int32_t MP_STDCALL Open(GmpiDrawing_API::IMpTessellationSink** returnObject) override
			{
				*returnObject = nullptr;

				auto sink = new TessellationSink(native_);
				auto r = sink->queryInterface(GmpiDrawing_API::SE_IID_TESSELLATIONSINK_MPGUI, reinterpret_cast<void **>(returnObject));

				return r;
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_MESH_MPGUI, GmpiDrawing_API::IMpMesh);
			GMPI_REFCOUNT;
		};

		class TextFormat : public GmpiDXWrapper<GmpiDrawing_API::IMpTextFormat, IDWriteTextFormat>
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>>* stringConverter; // constructed once is much faster.

		public:
			TextFormat(std::wstring_convert<std::codecvt_utf8<wchar_t>>* pstringConverter, IDWriteTextFormat* native) :
				GmpiDXWrapper<GmpiDrawing_API::IMpTextFormat, IDWriteTextFormat>(native)
				, stringConverter(pstringConverter)
			{}

			virtual int32_t MP_STDCALL SetTextAlignment(GmpiDrawing_API::MP1_TEXT_ALIGNMENT textAlignment) override
			{
				native()->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)textAlignment);
				return gmpi::MP_OK;
			}

			virtual int32_t MP_STDCALL SetParagraphAlignment(GmpiDrawing_API::MP1_PARAGRAPH_ALIGNMENT paragraphAlignment) override
			{
				native()->SetParagraphAlignment((DWRITE_PARAGRAPH_ALIGNMENT)paragraphAlignment);
				return gmpi::MP_OK;
			}

			virtual int32_t MP_STDCALL SetWordWrapping(GmpiDrawing_API::MP1_WORD_WRAPPING wordWrapping) override
			{
				return native()->SetWordWrapping((DWRITE_WORD_WRAPPING)wordWrapping);
			}

			virtual int32_t MP_STDCALL SetLineSpacing(float lineSpacing, float baseline) override
			{
				// For the default method, spacing depends solely on the content. For uniform spacing, the specified line height overrides the content.
				DWRITE_LINE_SPACING_METHOD method = lineSpacing < 0.0f ? DWRITE_LINE_SPACING_METHOD_DEFAULT : DWRITE_LINE_SPACING_METHOD_UNIFORM;
				return native()->SetLineSpacing(method, lineSpacing, baseline);
			}

			virtual int32_t MP_STDCALL GetFontMetrics(GmpiDrawing_API::MP1_FONT_METRICS* returnFontMetrics) override;

			// TODO!!!: Probably needs to accept constraint rect like DirectWrite. !!!
			//	virtual void MP_STDCALL GetTextExtentU(const char* utf8String, int32_t stringLength, GmpiDrawing::Size& returnSize)
			virtual void MP_STDCALL GetTextExtentU(const char* utf8String, int32_t stringLength, GmpiDrawing_API::MP1_SIZE* returnSize) override;

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_TEXTFORMAT_MPGUI, GmpiDrawing_API::IMpTextFormat);
			GMPI_REFCOUNT;
		};


		class bitmapPixels : public GmpiDrawing_API::IMpBitmapPixels
		{
			bool alphaPremultiplied;
			IWICBitmap* bitmap;
			UINT bytesPerRow;
			BYTE *ptr;
			IWICBitmapLock* pBitmapLock;
			ID2D1Bitmap* nativeBitmap_;
			int flags;

		public:
			bitmapPixels(ID2D1Bitmap* nativeBitmap, IWICBitmap* inBitmap, bool _alphaPremultiplied, int32_t pflags)
			{
				nativeBitmap_ = nativeBitmap;
				assert(inBitmap);

				UINT w, h;
				inBitmap->GetSize(&w, &h);

				bitmap = nullptr;
				pBitmapLock = nullptr;
				WICRect rcLock = { 0, 0, (INT)w, (INT)h };
				flags = pflags;

				if (0 <= inBitmap->Lock(&rcLock, flags, &pBitmapLock))
				{
					pBitmapLock->GetStride(&bytesPerRow);
					UINT bufferSize;
					pBitmapLock->GetDataPointer(&bufferSize, &ptr);

					bitmap = inBitmap;
					bitmap->AddRef();

					alphaPremultiplied = _alphaPremultiplied;
					if (!alphaPremultiplied)
						unpremultiplyAlpha();
				}
			}

			~bitmapPixels()
			{
				if (!alphaPremultiplied)
					premultiplyAlpha();

				if (nativeBitmap_)
				{
#if 1
					if (0 != (flags & GmpiDrawing_API::MP1_BITMAP_LOCK_WRITE))
					{
						D2D1_RECT_U r;
						r.left = r.top = 0;
						bitmap->GetSize(&r.right, &r.bottom);

						nativeBitmap_->CopyFromMemory(&r, ptr, bytesPerRow);
					}
#else
					nativeBitmap_->Release();
					nativeBitmap_ = nullptr;
#endif
				}

				SafeRelease(pBitmapLock);
				SafeRelease(bitmap);
			}

			virtual uint8_t* MP_STDCALL getAddress() const { return ptr; };
			virtual int32_t MP_STDCALL getBytesPerRow() const { return bytesPerRow; };
			virtual int32_t MP_STDCALL getPixelFormat() const { return kBGRA_SRGB; };

			inline uint8_t fast8bitScale(uint8_t a, uint8_t b)
			{
				int t = (int)a * (int)b;
				return (uint8_t)((t + 1 + (t >> 8)) >> 8); // fast way to divide by 255
			}

			void premultiplyAlpha()
			{
				UINT w, h;
				bitmap->GetSize(&w, &h);
				int totalPixels = h * bytesPerRow / sizeof(uint32_t);

				uint8_t* pixel = ptr;

				for (int i = 0; i < totalPixels; ++i)
				{
					if (pixel[3] == 0)
					{
						pixel[0] = 0;
						pixel[1] = 0;
						pixel[2] = 0;
					}
					else
					{
						pixel[0] = fast8bitScale(pixel[0], pixel[3]);
						pixel[1] = fast8bitScale(pixel[1], pixel[3]);
						pixel[2] = fast8bitScale(pixel[2], pixel[3]);
					}

					pixel += sizeof(uint32_t);
				}
			}

			//-----------------------------------------------------------------------------
			void unpremultiplyAlpha()
			{
				UINT w, h;
				bitmap->GetSize(&w, &h);
				int totalPixels = h * bytesPerRow / sizeof(uint32_t);

				uint8_t* pixel = ptr;

				for (int i = 0; i < totalPixels; ++i)
				{
					if (pixel[3] != 0)
					{
						pixel[0] = (uint32_t)(pixel[0] * 255) / pixel[3];
						pixel[1] = (uint32_t)(pixel[1] * 255) / pixel[3];
						pixel[2] = (uint32_t)(pixel[2] * 255) / pixel[3];
					}
					pixel += sizeof(uint32_t);
				}
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_BITMAP_PIXELS_MPGUI, GmpiDrawing_API::IMpBitmapPixels);
			GMPI_REFCOUNT;
		};

		class Bitmap : public GmpiDrawing_API::IMpBitmap
		{
		public:
			ID2D1Bitmap* nativeBitmap_;
			ID2D1RenderTarget* nativeContext_;
			IWICBitmap* diBitmap_;
			class Factory* factory;
#ifdef _DEBUG
			std::string debugFilename;
#endif
			Bitmap(Factory* pfactory, IWICBitmap* diBitmap) :
				nativeBitmap_(0)
				, nativeContext_(0)
				, diBitmap_(diBitmap)
				, factory(pfactory)
			{
				diBitmap->AddRef();

				ApplyPreMultiplyCorrection();
			}

			Bitmap(Factory* pfactory, ID2D1RenderTarget* nativeContext, ID2D1Bitmap* nativeBitmap) :
				nativeBitmap_(nativeBitmap)
				, nativeContext_(nativeContext)
				, diBitmap_(nullptr)
				, factory(pfactory)
			{
				nativeBitmap->AddRef();
			}

			~Bitmap()
			{
				//		AFX_MANAGE_STATE(AfxGetStaticModuleState());

				if (nativeBitmap_)
				{
					nativeBitmap_->Release();
				}
				if (diBitmap_)
				{
					diBitmap_->Release();
				}
			}

			ID2D1Bitmap* GetNativeBitmap(class GraphicsContext* context);

			virtual GmpiDrawing_API::MP1_SIZE MP_STDCALL GetSizeF() override
			{
				GmpiDrawing_API::MP1_SIZE returnSize;
				UINT w, h;
				diBitmap_->GetSize(&w, &h);
				returnSize.width = (float)w;
				returnSize.height = (float)h;

				return returnSize;
			}

			virtual int32_t MP_STDCALL GetSize(GmpiDrawing_API::MP1_SIZE_U* returnSize) override
			{
				diBitmap_->GetSize(&returnSize->width, &returnSize->height);

				return gmpi::MP_OK;
			}

			virtual int32_t MP_STDCALL lockPixelsOld(GmpiDrawing_API::IMpBitmapPixels** returnInterface, bool alphaPremultiplied) override
			{
				*returnInterface = 0;

				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new bitmapPixels(nativeBitmap_, diBitmap_, alphaPremultiplied, GmpiDrawing_API::MP1_BITMAP_LOCK_READ | GmpiDrawing_API::MP1_BITMAP_LOCK_WRITE));

				return b2->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_PIXELS_MPGUI, (void**)(returnInterface));
			}

			virtual int32_t MP_STDCALL lockPixels(GmpiDrawing_API::IMpBitmapPixels** returnInterface, int32_t flags);

			void MP_STDCALL ApplyAlphaCorrection() override{}; // deprecated
			void ApplyAlphaCorrection_win7();
			void ApplyPreMultiplyCorrection();

			virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory** pfactory) override;

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_BITMAP_MPGUI, GmpiDrawing_API::IMpBitmap);
			GMPI_REFCOUNT;
		};


		class GeometrySink : public GmpiDrawing_API::IMpGeometrySink
		{
			ID2D1GeometrySink* geometrysink_;

		public:
			GeometrySink(ID2D1GeometrySink* context) : geometrysink_(context) {}
			~GeometrySink()
			{
				if (geometrysink_)
				{
					geometrysink_->Release();
				}
			}
			virtual void MP_STDCALL SetFillMode(GmpiDrawing_API::MP1_FILL_MODE fillMode)
			{
				geometrysink_->SetFillMode((D2D1_FILL_MODE)fillMode);
			}
			virtual void MP_STDCALL SetSegmentFlags(GmpiDrawing_API::MP1_PATH_SEGMENT vertexFlags)
			{
				geometrysink_->SetSegmentFlags((D2D1_PATH_SEGMENT)vertexFlags);
			}
			virtual void MP_STDCALL BeginFigure(GmpiDrawing_API::MP1_POINT startPoint, GmpiDrawing_API::MP1_FIGURE_BEGIN figureBegin) override
			{
				geometrysink_->BeginFigure(*reinterpret_cast<D2D1_POINT_2F*>(&startPoint), (D2D1_FIGURE_BEGIN)figureBegin);
			}
			virtual void MP_STDCALL AddLines(const GmpiDrawing_API::MP1_POINT* points, uint32_t pointsCount) override
			{
				geometrysink_->AddLines(reinterpret_cast<const D2D1_POINT_2F*>(points), pointsCount);
			}
			virtual void MP_STDCALL AddBeziers(const GmpiDrawing_API::MP1_BEZIER_SEGMENT* beziers, uint32_t beziersCount) override
			{
				geometrysink_->AddBeziers(reinterpret_cast<const D2D1_BEZIER_SEGMENT*>(beziers), beziersCount);
			}
			virtual void MP_STDCALL EndFigure(GmpiDrawing_API::MP1_FIGURE_END figureEnd) override
			{
				geometrysink_->EndFigure((D2D1_FIGURE_END)figureEnd);
			}
			virtual int32_t MP_STDCALL Close() override
			{
				auto hr = geometrysink_->Close();
				return hr == 0 ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
			}
			virtual void MP_STDCALL AddLine(GmpiDrawing_API::MP1_POINT point) override
			{
				geometrysink_->AddLine(*reinterpret_cast<D2D1_POINT_2F*>(&point));
			}
			virtual void MP_STDCALL AddBezier(const GmpiDrawing_API::MP1_BEZIER_SEGMENT* bezier) override
			{
				geometrysink_->AddBezier(reinterpret_cast<const D2D1_BEZIER_SEGMENT*>(bezier));
			}
			virtual void MP_STDCALL AddQuadraticBezier(const GmpiDrawing_API::MP1_QUADRATIC_BEZIER_SEGMENT* bezier) override
			{
				geometrysink_->AddQuadraticBezier(reinterpret_cast<const D2D1_QUADRATIC_BEZIER_SEGMENT*>(bezier));
			}
			virtual void MP_STDCALL AddQuadraticBeziers(const GmpiDrawing_API::MP1_QUADRATIC_BEZIER_SEGMENT* beziers, uint32_t beziersCount) override
			{
				geometrysink_->AddQuadraticBeziers(reinterpret_cast<const D2D1_QUADRATIC_BEZIER_SEGMENT*>(beziers), beziersCount);
			}
			virtual void MP_STDCALL AddArc(const GmpiDrawing_API::MP1_ARC_SEGMENT* arc) override
			{
				geometrysink_->AddArc(reinterpret_cast<const D2D1_ARC_SEGMENT*>(arc));
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_GEOMETRYSINK_MPGUI, GmpiDrawing_API::IMpGeometrySink);
			GMPI_REFCOUNT;
		};


		class Geometry : public GmpiDrawing_API::IMpPathGeometry
		{
			friend class GraphicsContext;

			ID2D1PathGeometry* geometry_;

		public:
			Geometry(ID2D1PathGeometry* context) : geometry_(context)
			{}
			~Geometry()
			{
				if (geometry_)
				{
					geometry_->Release();
				}
			}
			virtual int32_t MP_STDCALL Open(GmpiDrawing_API::IMpGeometrySink** geometrySink) override;
			virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory** factory) override
			{
				//		native_->GetFactory((ID2D1Factory**)factory);
			}

			virtual int32_t MP_STDCALL StrokeContainsPoint(GmpiDrawing_API::MP1_POINT point, float strokeWidth, GmpiDrawing_API::IMpStrokeStyle* strokeStyle, const GmpiDrawing_API::MP1_MATRIX_3X2* worldTransform, bool* returnContains) override
			{
				BOOL result = FALSE;
				geometry_->StrokeContainsPoint(*(D2D1_POINT_2F*)&point, strokeWidth, toNative(strokeStyle), (const D2D1_MATRIX_3X2_F *)worldTransform, &result);
				*returnContains = result == TRUE;

				return gmpi::MP_OK;
			}
			virtual int32_t MP_STDCALL FillContainsPoint(GmpiDrawing_API::MP1_POINT point, const GmpiDrawing_API::MP1_MATRIX_3X2* worldTransform, bool* returnContains) override
			{
				BOOL result = FALSE;
				geometry_->FillContainsPoint(*(D2D1_POINT_2F*)&point, (const D2D1_MATRIX_3X2_F *)worldTransform, &result);
				*returnContains = result == TRUE;

				return gmpi::MP_OK;
			}
			virtual int32_t MP_STDCALL GetWidenedBounds(float strokeWidth, GmpiDrawing_API::IMpStrokeStyle* strokeStyle, const GmpiDrawing_API::MP1_MATRIX_3X2* worldTransform, GmpiDrawing_API::MP1_RECT* returnBounds) override
			{
				geometry_->GetWidenedBounds(strokeWidth, toNative(strokeStyle), (const D2D1_MATRIX_3X2_F *)worldTransform, (D2D_RECT_F*)returnBounds);
				return gmpi::MP_OK;
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_PATHGEOMETRY_MPGUI, GmpiDrawing_API::IMpPathGeometry);
			GMPI_REFCOUNT;
		};


		class Factory : public GmpiDrawing_API::IMpFactory
		{
			ID2D1Factory1* m_pDirect2dFactory;
			IDWriteFactory* writeFactory;
			IWICImagingFactory* pIWICFactory;
			std::vector<std::wstring> supportedFontFamilies;
			std::map<std::wstring, std::wstring> GdiFontConversions;
			bool DX_support_sRGB;
		public:
			std::wstring_convert<std::codecvt_utf8<wchar_t>> stringConverter; // cached, as constructor is super-slow.
			void setSrgbSupport(bool s)
			{
				DX_support_sRGB = s;
			}

			Factory();
			void Init(ID2D1Factory1* existingFactory = nullptr);
			~Factory();

			ID2D1Factory1* getD2dFactory()
			{
				return m_pDirect2dFactory;
			}
			std::wstring fontMatch(std::wstring fontName, GmpiDrawing_API::MP1_FONT_WEIGHT fontWeight, float fontSize);

			virtual int32_t MP_STDCALL CreatePathGeometry(GmpiDrawing_API::IMpPathGeometry** pathGeometry) override;
			virtual int32_t MP_STDCALL CreateTextFormat(const char* fontFamilyName, void* unused /* fontCollection */, GmpiDrawing_API::MP1_FONT_WEIGHT fontWeight, GmpiDrawing_API::MP1_FONT_STYLE fontStyle, GmpiDrawing_API::MP1_FONT_STRETCH fontStretch, float fontSize, void* unused2 /* localeName */, GmpiDrawing_API::IMpTextFormat** textFormat) override;
			virtual int32_t MP_STDCALL CreateImage(int32_t width, int32_t height, GmpiDrawing_API::IMpBitmap** returnDiBitmap) override;
			virtual int32_t MP_STDCALL LoadImageU(const char* utf8Uri, GmpiDrawing_API::IMpBitmap** returnDiBitmap) override;
			virtual int32_t CreateStrokeStyle(const GmpiDrawing_API::MP1_STROKE_STYLE_PROPERTIES* strokeStyleProperties, float* dashes, int32_t dashesCount, GmpiDrawing_API::IMpStrokeStyle** returnValue) override
			{
				*returnValue = nullptr;

				ID2D1StrokeStyle* b = nullptr;

				auto hr = m_pDirect2dFactory->CreateStrokeStyle((const D2D1_STROKE_STYLE_PROPERTIES*) strokeStyleProperties, dashes, dashesCount, &b);

				if (hr == 0)
				{
					gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> wrapper;
					wrapper.Attach(new StrokeStyle(b, this));

//					auto wrapper = gmpi_sdk::make_shared_ptr<StrokeStyle>(b, this);

					wrapper->queryInterface(GmpiDrawing_API::SE_IID_STROKESTYLE_MPGUI, reinterpret_cast<void**>(returnValue));
				}

				return hr == 0 ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
			}

			IWICBitmap* CreateDiBitmapFromNative(ID2D1Bitmap* D2D_Bitmap);

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_FACTORY_MPGUI, GmpiDrawing_API::IMpFactory);
			GMPI_REFCOUNT_NO_DELETE;
		};


		class GraphicsContext : public GmpiDrawing_API::IMpDeviceContext
		{
		protected:
			ID2D1RenderTarget* context_;
			ID2D1DeviceContext* context2_ = nullptr; // extended functionality. Not always available.

			Factory* factory;
			std::vector<GmpiDrawing_API::MP1_RECT> clipRectStack;
			std::wstring_convert<std::codecvt_utf8<wchar_t>>* stringConverter; // cached, as constructor is super-slow.
			bool mSupportSRGB;

			void Init()
			{
				const float defaultClipBounds = 100000.0f;
				GmpiDrawing_API::MP1_RECT r;
				r.top = r.left = -defaultClipBounds;
				r.bottom = r.right = defaultClipBounds;
				clipRectStack.push_back(r);

				stringConverter = &(factory->stringConverter);
			}

		public:
			GraphicsContext(ID2D1DeviceContext* deviceContext, Factory* pfactory) :
				context_(deviceContext)
				, context2_(deviceContext)
				, factory(pfactory),
				mSupportSRGB(true)
			{
				context_->AddRef();
				Init();
			}

			GraphicsContext(ID2D1RenderTarget* context, Factory* pfactory) :
				context_(context)
				, factory(pfactory),
				mSupportSRGB(true)
			{
				context_->AddRef();
				Init();
			}

			GraphicsContext(Factory* pfactory) :
				context_(nullptr)
				, factory(pfactory),
				mSupportSRGB(true)
			{
				Init();
			}

			~GraphicsContext()
			{
				context_->Release();
			}

			ID2D1RenderTarget* native()
			{
				return context_;
			}

			virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory** pfactory) override
			{
				*pfactory = factory;
			}

			virtual void MP_STDCALL DrawRectangle(const GmpiDrawing_API::MP1_RECT* rect, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle) override
			{
				context_->DrawRectangle(D2D1::RectF(rect->left, rect->top, rect->right, rect->bottom), ((Brush*)brush)->nativeBrush(), strokeWidth, toNative(strokeStyle) );
			}

			virtual void MP_STDCALL FillRectangle(const GmpiDrawing_API::MP1_RECT* rect, const GmpiDrawing_API::IMpBrush* brush) override
			{
				context_->FillRectangle((D2D1_RECT_F*)rect, (ID2D1Brush*)((Brush*)brush)->nativeBrush());
			}

			void MP_STDCALL Clear(const GmpiDrawing_API::MP1_COLOR* clearColor) override
			{
				context_->Clear((D2D1_COLOR_F*)clearColor);
			}

			virtual void MP_STDCALL DrawLine(GmpiDrawing_API::MP1_POINT point0, GmpiDrawing_API::MP1_POINT point1, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle) override
			{
				context_->DrawLine(*((D2D_POINT_2F*)&point0), *((D2D_POINT_2F*)&point1), ((Brush*)brush)->nativeBrush(), strokeWidth, toNative(strokeStyle));
			}

			virtual void MP_STDCALL DrawGeometry(const GmpiDrawing_API::IMpPathGeometry* geometry, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth = 1.0f, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle = 0) override;

			virtual void MP_STDCALL FillGeometry(const GmpiDrawing_API::IMpPathGeometry* geometry, const GmpiDrawing_API::IMpBrush* brush, const GmpiDrawing_API::IMpBrush* opacityBrush) override
			{
				auto d2d_geometry = ((Geometry*)geometry)->geometry_;

				ID2D1Brush* opacityBrushNative;
				if (opacityBrush)
				{
					opacityBrushNative = ((Brush*)brush)->nativeBrush();
				}
				else
				{
					opacityBrushNative = nullptr;
				}

				context_->FillGeometry(d2d_geometry, ((Brush*)brush)->nativeBrush(), opacityBrushNative);
			}

			//void MP_STDCALL FillMesh(const GmpiDrawing_API::IMpMesh* mesh, const GmpiDrawing_API::IMpBrush* brush) override
			//{
			//	auto nativeMesh = ((Mesh*)mesh)->native();
			//	context_->FillMesh(nativeMesh, ((Brush*)brush)->nativeBrush());
			//}

			virtual void MP_STDCALL DrawTextU(const char* utf8String, int32_t stringLength, const GmpiDrawing_API::IMpTextFormat* textFormat, const GmpiDrawing_API::MP1_RECT* layoutRect, const GmpiDrawing_API::IMpBrush* brush, int32_t flags) override;

			//	virtual void MP_STDCALL DrawBitmap( GmpiDrawing_API::IMpBitmap* mpBitmap, GmpiDrawing::Rect destinationRectangle, float opacity, int32_t interpolationMode, GmpiDrawing::Rect sourceRectangle) override
			virtual void MP_STDCALL DrawBitmap(const GmpiDrawing_API::IMpBitmap* mpBitmap, const GmpiDrawing_API::MP1_RECT* destinationRectangle, float opacity, /* MP1_BITMAP_INTERPOLATION_MODE*/ int32_t interpolationMode, const GmpiDrawing_API::MP1_RECT* sourceRectangle) override
			{
				auto bm = ((Bitmap*)mpBitmap);
				auto bitmap = bm->GetNativeBitmap(this);
				if (bitmap)
				{
					// D2D1_BITMAP_INTERPOLATION_MODE interpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
					context_->DrawBitmap(bitmap,
						(D2D1_RECT_F*)destinationRectangle,
						opacity, /*D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, //*/(D2D1_BITMAP_INTERPOLATION_MODE)interpolationMode,
						(D2D1_RECT_F*)sourceRectangle);
				}
			}

			virtual void MP_STDCALL SetTransform(const GmpiDrawing_API::MP1_MATRIX_3X2* transform) override
			{
				context_->SetTransform(reinterpret_cast<const D2D1_MATRIX_3X2_F*>(transform));
			}

			virtual void MP_STDCALL GetTransform(GmpiDrawing_API::MP1_MATRIX_3X2* transform) override
			{
				context_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
			}

			virtual int32_t MP_STDCALL CreateSolidColorBrush(const GmpiDrawing_API::MP1_COLOR* color, GmpiDrawing_API::IMpSolidColorBrush **solidColorBrush) override;

			virtual int32_t MP_STDCALL CreateGradientStopCollection(const GmpiDrawing_API::MP1_GRADIENT_STOP* gradientStops, uint32_t gradientStopsCount, /* GmpiDrawing_API::MP1_GAMMA colorInterpolationGamma, GmpiDrawing_API::MP1_EXTEND_MODE extendMode,*/ GmpiDrawing_API::IMpGradientStopCollection** gradientStopCollection) override;

			int32_t MP_STDCALL CreateLinearGradientBrush(const GmpiDrawing_API::MP1_LINEAR_GRADIENT_BRUSH_PROPERTIES* linearGradientBrushProperties, const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties, const  GmpiDrawing_API::IMpGradientStopCollection* gradientStopCollection, GmpiDrawing_API::IMpLinearGradientBrush** linearGradientBrush) override
			{
				*linearGradientBrush = nullptr;
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new LinearGradientBrush(factory, context_, linearGradientBrushProperties, brushProperties, gradientStopCollection));
				return b2->queryInterface(GmpiDrawing_API::SE_IID_LINEARGRADIENTBRUSH_MPGUI, reinterpret_cast<void **>(linearGradientBrush));
			}
			//	virtual int32_t MP_STDCALL CreateBitmap(GmpiDrawing_API::MP1_SIZE_U size, const GmpiDrawing_API::MP1_BITMAP_PROPERTIES* bitmapProperties, GmpiDrawing_API::IMpBitmap** bitmap) override;

			virtual int32_t MP_STDCALL CreateBitmapBrush(const GmpiDrawing_API::IMpBitmap* bitmap, const GmpiDrawing_API::MP1_BITMAP_BRUSH_PROPERTIES* bitmapBrushProperties, const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties, GmpiDrawing_API::IMpBitmapBrush** bitmapBrush) override
			{
				return context_->CreateBitmapBrush((ID2D1Bitmap*)bitmap, (D2D1_BITMAP_BRUSH_PROPERTIES*)bitmapBrushProperties, (D2D1_BRUSH_PROPERTIES*)brushProperties, (ID2D1BitmapBrush**)bitmapBrush);
			}
			virtual int32_t MP_STDCALL CreateRadialGradientBrush(const GmpiDrawing_API::MP1_RADIAL_GRADIENT_BRUSH_PROPERTIES* radialGradientBrushProperties, const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties, const GmpiDrawing_API::IMpGradientStopCollection* gradientStopCollection, GmpiDrawing_API::IMpRadialGradientBrush** radialGradientBrush) override
			{
				return context_->CreateRadialGradientBrush((D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES*)radialGradientBrushProperties, (D2D1_BRUSH_PROPERTIES*)brushProperties, (ID2D1GradientStopCollection*)gradientStopCollection, (ID2D1RadialGradientBrush**)radialGradientBrush);
			}

			virtual int32_t MP_STDCALL CreateCompatibleRenderTarget(const GmpiDrawing_API::MP1_SIZE* desiredSize, GmpiDrawing_API::IMpBitmapRenderTarget** bitmapRenderTarget) override;

			virtual void MP_STDCALL DrawRoundedRectangle(const GmpiDrawing_API::MP1_ROUNDED_RECT* roundedRect, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle) override
			{
				context_->DrawRoundedRectangle((D2D1_ROUNDED_RECT*)roundedRect, (ID2D1Brush*)((Brush*)brush)->nativeBrush(), (FLOAT)strokeWidth, toNative(strokeStyle));
			}

//			int32_t MP_STDCALL CreateMesh(GmpiDrawing_API::IMpMesh** returnObject) override;

			virtual void MP_STDCALL FillRoundedRectangle(const GmpiDrawing_API::MP1_ROUNDED_RECT* roundedRect, const GmpiDrawing_API::IMpBrush* brush) override
			{
				context_->FillRoundedRectangle((D2D1_ROUNDED_RECT*)roundedRect, (ID2D1Brush*)((Brush*)brush)->nativeBrush());
			}

			virtual void MP_STDCALL DrawEllipse(const GmpiDrawing_API::MP1_ELLIPSE* ellipse, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle) override
			{
				context_->DrawEllipse((D2D1_ELLIPSE*)ellipse, (ID2D1Brush*)((Brush*)brush)->nativeBrush(), (FLOAT)strokeWidth, toNative(strokeStyle));
			}

			virtual void MP_STDCALL FillEllipse(const GmpiDrawing_API::MP1_ELLIPSE* ellipse, const GmpiDrawing_API::IMpBrush* brush) override
			{
				context_->FillEllipse((D2D1_ELLIPSE*)ellipse, (ID2D1Brush*)((Brush*)brush)->nativeBrush());
			}

			virtual void MP_STDCALL PushAxisAlignedClip(const GmpiDrawing_API::MP1_RECT* clipRect/*, GmpiDrawing_API::MP1_ANTIALIAS_MODE antialiasMode*/) override;

			virtual void MP_STDCALL PopAxisAlignedClip() override
			{
//				_RPT0(_CRT_WARN, "                 PopAxisAlignedClip()\n");
				context_->PopAxisAlignedClip();
				clipRectStack.pop_back();
			}

			virtual void MP_STDCALL GetAxisAlignedClip(GmpiDrawing_API::MP1_RECT* returnClipRect) override;

			virtual void MP_STDCALL BeginDraw() override
			{
				context_->BeginDraw();
			}

			virtual int32_t MP_STDCALL EndDraw() override
			{
				auto hr = context_->EndDraw();

				return hr == S_OK ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
			}
			/*
				virtual int32_t MP_STDCALL GetUpdateRegion(GmpiDrawing_API::IUpdateRegion** returnUpdateRegion) override
				{
					*returnUpdateRegion = updateRegion_;
					return gmpi::MP_OK;
				}
			*/

			//	virtual void MP_STDCALL InsetNewMethodHere(){}

			//////////////////////////////////////////////
			bool SupportSRGB()
			{
				return mSupportSRGB;
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_DEVICECONTEXT_MPGUI, GmpiDrawing_API::IMpDeviceContext);
			GMPI_REFCOUNT_NO_DELETE;
		};

		class BitmapRenderTarget : public GraphicsContext
		{
		public:
			BitmapRenderTarget(GraphicsContext* g, const GmpiDrawing_API::MP1_SIZE* desiredSize, Factory* pfactory) :
				GraphicsContext(pfactory)
			{
				/* auto hr = */ g->native()->CreateCompatibleRenderTarget(*(D2D1_SIZE_F*)desiredSize, (ID2D1BitmapRenderTarget**) &context_);
				mSupportSRGB = g->SupportSRGB();
			}

			virtual int32_t MP_STDCALL GetBitmap(GmpiDrawing_API::IMpBitmap** returnBitmap);

			virtual int32_t MP_STDCALL queryInterface(const gmpi::MpGuid& iid, void** returnInterface) override
			{
				*returnInterface = 0;
				if (iid == GmpiDrawing_API::SE_IID_BITMAP_RENDERTARGET_MPGUI)
				{
					// non-standard. Forcing this class (which has the correct vtable) to pretend it's the emulated interface.
					*returnInterface = reinterpret_cast<GmpiDrawing_API::IMpBitmapRenderTarget*>(this);
					addRef();
					return gmpi::MP_OK;
				}
				else
				{
					return GraphicsContext::queryInterface(iid, returnInterface);
				}
			}

			GMPI_REFCOUNT;
		};

		// Direct2D context tailored to devices without sRGB high-color support. i.e. Windows 7.
		class GraphicsContext_Win7 : public GraphicsContext
		{
		public:

			GraphicsContext_Win7(ID2D1RenderTarget* context, Factory* pfactory) :
				GraphicsContext(context, pfactory)
			{}

			int32_t MP_STDCALL CreateSolidColorBrush(const GmpiDrawing_API::MP1_COLOR* color, GmpiDrawing_API::IMpSolidColorBrush **solidColorBrush) override
			{
				*solidColorBrush = nullptr;

				auto brush = new SolidColorBrush_Win7(context_, color, factory);
				return brush->queryInterface(GmpiDrawing_API::SE_IID_SOLIDCOLORBRUSH_MPGUI, reinterpret_cast<void **>(solidColorBrush));
			}

			int32_t MP_STDCALL CreateLinearGradientBrush(const GmpiDrawing_API::MP1_LINEAR_GRADIENT_BRUSH_PROPERTIES* linearGradientBrushProperties, const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties, const  GmpiDrawing_API::IMpGradientStopCollection* gradientStopCollection, GmpiDrawing_API::IMpLinearGradientBrush** linearGradientBrush) override
			{
				*linearGradientBrush = nullptr;
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new LinearGradientBrush_win7(factory, context_, linearGradientBrushProperties, brushProperties, gradientStopCollection));
				return b2->queryInterface(GmpiDrawing_API::SE_IID_LINEARGRADIENTBRUSH_MPGUI, reinterpret_cast<void **>(linearGradientBrush));
			}

			void MP_STDCALL Clear(const GmpiDrawing_API::MP1_COLOR* clearColor) override
			{
				GmpiDrawing_API::MP1_COLOR color(*clearColor);
				color.r = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color.r));
				color.g = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color.g));
				color.b = se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color.b));
				context_->Clear((D2D1_COLOR_F*)&color);
			}
		};


	} // Namespace
} // Namespace