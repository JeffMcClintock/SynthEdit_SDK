#pragma once

/*
#include "Cocoa_Gfx.h"
*/

#include <codecvt>
#include <map>
#include "../se_sdk3/Drawing.h"
#include "../shared/xp_simd.h"
#include "./Gfx_base.h"

namespace gmpi
{
	namespace cocoa
	{
		// Conversion utilities.
		inline GmpiDrawing::Rect RectFromNSRect(NSRect nsr)
		{
			GmpiDrawing::Rect r(nsr.origin.x, nsr.origin.y, nsr.origin.x + nsr.size.width, nsr.origin.y + nsr.size.height);
			return r;
		}

		inline NSRect NSRectFromRect(GmpiDrawing_API::MP1_RECT rect)
		{
			return NSMakeRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
		}
        
        // helper
        void SetNativePenStrokeStyle(NSBezierPath * path, GmpiDrawing_API::IMpStrokeStyle* strokeStyle)
        {
            GmpiDrawing_API::MP1_CAP_STYLE capstyle = strokeStyle == nullptr ? GmpiDrawing_API::MP1_CAP_STYLE_FLAT : strokeStyle->GetStartCap();
            
            switch(capstyle)
            {
                default:
                case GmpiDrawing_API::MP1_CAP_STYLE_FLAT:
                    [ path setLineCapStyle:NSButtLineCapStyle ];
                    break;
                    
                case GmpiDrawing_API::MP1_CAP_STYLE_SQUARE:
                    [ path setLineCapStyle:NSSquareLineCapStyle ];
                    break;
                    
                case GmpiDrawing_API::MP1_CAP_STYLE_TRIANGLE:
                case GmpiDrawing_API::MP1_CAP_STYLE_ROUND:
                    [ path setLineCapStyle:NSRoundLineCapStyle ];
                    break;
            }
        }

		// Classes without GetFactory()
		template<class MpInterface, class CocoaType>
		class CocoaWrapper : public MpInterface
		{
		protected:
			CocoaType* native_;

			virtual ~CocoaWrapper()
			{
				if (native_)
				{
					[native_ release];
				}
			}

		public:
			CocoaWrapper(CocoaType* native) : native_(native) {}

			inline CocoaType* native()
			{
				return native_;
			}

			GMPI_REFCOUNT;
		};

		// Classes with GetFactory()
		template<class MpInterface, class CocoaType>
		class CocoaWrapperWithFactory : public CocoaWrapper<MpInterface, CocoaType>
		{
		protected:
			GmpiDrawing_API::IMpFactory* factory_;

		public:
			CocoaWrapperWithFactory(CocoaType* native, GmpiDrawing_API::IMpFactory* factory) : CocoaWrapper<MpInterface, CocoaType>(native), factory_(factory) {}

			virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory **factory) override
			{
				*factory = factory_;
			}
		};

		class nothing
		{

		};
/*
		class Brush : / * public GmpiDrawing_API::IMpBrush,* / public CocoaWrapperWithFactory<GmpiDrawing_API::IMpBrush, nothing> // Resource
		{
		public:
			Brush(GmpiDrawing_API::IMpFactory* factory) : CocoaWrapperWithFactory(nullptr, factory) {}
		};
*/
        
        class CocoaBrushBase
        {
		protected:
			GmpiDrawing_API::IMpFactory* factory_;

        public:
			CocoaBrushBase(GmpiDrawing_API::IMpFactory* pfactory) : factory_(pfactory){}
            virtual ~CocoaBrushBase(){}
            
			virtual void FillPath(NSBezierPath* nsPath) const = 0;

            // Default to black fill for fancy brushes that don't implement line drawing yet.
            virtual void StrokePath(NSBezierPath* nsPath, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle = nullptr) const
            {
                [[NSColor blackColor] set]; /// !!!TODO!!!, color set to black always.
                
                [nsPath setLineWidth : strokeWidth];
                SetNativePenStrokeStyle(nsPath, (GmpiDrawing_API::IMpStrokeStyle*) strokeStyle);
                
                [nsPath stroke];
            }
    };

		class SolidColorBrush : public GmpiDrawing_API::IMpSolidColorBrush, public CocoaBrushBase
		{
			GmpiDrawing_API::MP1_COLOR color;
			NSColor* nativec_ = nullptr;
            
            inline void setNativeColor()
            {
                //nativec_ = [NSColor colorWithSRGBRed : (CGFloat)color.r green : (CGFloat)color.g blue : (CGFloat)color.b alpha : (CGFloat)color.a];
                //                nativec_ = [NSColor colorWithDeviceRed : (CGFloat)color.r green : (CGFloat)color.g blue : (CGFloat)color.b alpha : (CGFloat)color.a];
                
                nativec_ = [NSColor
                            colorWithDeviceRed : (CGFloat)se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color.r))
                            green : (CGFloat)se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color.g))
                            blue : (CGFloat)se_sdk::FastGamma::pixelToNormalised(se_sdk::FastGamma::float_to_sRGB(color.b))
                            alpha : (CGFloat)color.a];
            }
            
		public:
			SolidColorBrush(const GmpiDrawing_API::MP1_COLOR* pcolor, GmpiDrawing_API::IMpFactory *factory) : CocoaBrushBase(factory)
				,color(*pcolor)
			{
                setNativeColor();
			}

            inline NSColor* nativeColor() const
            {
                return nativec_;
            }

            virtual void FillPath(NSBezierPath* nsPath) const override
            {
                [nativec_ set];
                [nsPath fill];
            }

			virtual void StrokePath(NSBezierPath* nsPath, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle = nullptr) const override
			{
				[nativec_ set];
				[nsPath setLineWidth : strokeWidth];
                SetNativePenStrokeStyle(nsPath, (GmpiDrawing_API::IMpStrokeStyle*) strokeStyle);
                
                [nsPath stroke];
			}

			~SolidColorBrush()
			{
				// crash       [nativec_ release];
			}


			// IMPORTANT: Virtual functions much 100% match GmpiDrawing_API::IMpSolidColorBrush to simulate inheritance.
			virtual void MP_STDCALL SetColor(const GmpiDrawing_API::MP1_COLOR* pcolor) override
			{
				color = *pcolor;
                setNativeColor();
			}

			virtual GmpiDrawing_API::MP1_COLOR MP_STDCALL GetColor() override
			{
				return color;
			}

            virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory **factory) override
            {
                *factory = factory_;
            }
            
            GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_SOLIDCOLORBRUSH_MPGUI, GmpiDrawing_API::IMpSolidColorBrush);
			GMPI_REFCOUNT;
		};

		class GradientStopCollection : public CocoaWrapperWithFactory<GmpiDrawing_API::IMpGradientStopCollection, nothing>
		{
		public:
			std::vector<GmpiDrawing_API::MP1_GRADIENT_STOP> gradientstops;

			GradientStopCollection(GmpiDrawing_API::IMpFactory* factory, const GmpiDrawing_API::MP1_GRADIENT_STOP* gradientStops, uint32_t gradientStopsCount) : CocoaWrapperWithFactory(nullptr, factory)
			{
				for (uint32_t i = 0; i < gradientStopsCount; ++i)
				{
					gradientstops.push_back(gradientStops[i]);
				}
			}
			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_GRADIENTSTOPCOLLECTION_MPGUI, GmpiDrawing_API::IMpGradientStopCollection);
		};


		//class LinearGradientBrush : /* Simulated: public GmpiDrawing_API::IMpLinearGradientBrush,*/ public Brush
		class LinearGradientBrush : public GmpiDrawing_API::IMpLinearGradientBrush, public CocoaBrushBase
		{
			NSGradient* native2;
			GmpiDrawing::Point startPoint;
			GmpiDrawing::Point endPoint;
		public:

			LinearGradientBrush(GmpiDrawing_API::IMpFactory *factory, const GmpiDrawing_API::MP1_LINEAR_GRADIENT_BRUSH_PROPERTIES* linearGradientBrushProperties, const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties, const  GmpiDrawing_API::IMpGradientStopCollection* gradientStopCollection) : CocoaBrushBase(factory)
				, startPoint(0, 0)
				, endPoint(0, 1)
			{
				auto stops = static_cast<const GradientStopCollection*>(gradientStopCollection);

				NSMutableArray* colors = [NSMutableArray array];
				std::vector<CGFloat> locations2;

				for (auto& stop : stops->gradientstops)
				{
					[colors addObject : [NSColor /*colorWithRed*/ colorWithDeviceRed : stop.color.r green : stop.color.g blue : stop.color.b alpha : stop.color.a]];
					locations2.push_back(stop.position);
				}

				native2 = [[NSGradient alloc] initWithColors:colors atLocations : locations2.data() colorSpace : [NSColorSpace sRGBColorSpace]];
			}

			inline NSGradient* native() const
			{
				return native2;
			}

			float getAngle() const
			{
				// TODO cache. e.g. nan = not calculated yet.
				return (180.0f / M_PI) * atan2((endPoint.y - startPoint.y), (endPoint.x - startPoint.x));
			}

			// IMPORTANT: Virtual functions much 100% match simulated interface.
			virtual void MP_STDCALL SetStartPoint(GmpiDrawing_API::MP1_POINT pstartPoint) override
			{
				startPoint = pstartPoint;
			}
			virtual void MP_STDCALL SetEndPoint(GmpiDrawing_API::MP1_POINT pendPoint) override
			{
				endPoint = pendPoint;
			}

			virtual void FillPath(NSBezierPath* nsPath) const override
			{
				[native2 drawInBezierPath:nsPath angle : getAngle()];
			}
/* TODO (convert to outline, use fill functions).
			virtual void StrokePath(NSBezierPath* nsPath, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle = nullptr) const override
			{
				[[NSColor blackColor] set]; /// !!!TODO!!!, color set to black always.

				[nsPath setLineWidth : strokeWidth];
				[nsPath stroke];
			}
*.
			/*
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
			*/
            
            virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory **factory) override
            {
                *factory = factory_;
            }

			GMPI_REFCOUNT;
			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_LINEARGRADIENTBRUSH_MPGUI, GmpiDrawing_API::IMpLinearGradientBrush);
		};

		class RadialGradientBrush : public GmpiDrawing_API::IMpRadialGradientBrush, public CocoaBrushBase
		{
			NSGradient* native2;
			GmpiDrawing::Point startPoint;
			GmpiDrawing::Point endPoint;
			GmpiDrawing::Size radius;
			GmpiDrawing::Point center;

		public:
			RadialGradientBrush(GmpiDrawing_API::IMpFactory *factory, const GmpiDrawing_API::MP1_RADIAL_GRADIENT_BRUSH_PROPERTIES* radialGradientBrushProperties, const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties, const  GmpiDrawing_API::IMpGradientStopCollection* gradientStopCollection) : CocoaBrushBase(factory)
				, startPoint(0, 0)
				, endPoint(0, 1)
			{
				auto stops = static_cast<const GradientStopCollection*>(gradientStopCollection);

				NSMutableArray* colors = [NSMutableArray array];
				std::vector<CGFloat> locations2;

				for (auto& stop : stops->gradientstops)
				{
					[colors addObject : [NSColor /*colorWithRed*/ colorWithDeviceRed : stop.color.r green : stop.color.g blue : stop.color.b alpha : stop.color.a]];
					locations2.push_back(stop.position);
				}

				native2 = [[NSGradient alloc] initWithColors:colors atLocations : locations2.data() colorSpace : [NSColorSpace sRGBColorSpace]];
			}

			virtual void FillPath(NSBezierPath* nsPath) const override
			{
//				[native2 drawInBezierPath : nsPath angle : getAngle()];

				[native2 drawFromCenter: NSMakePoint(center.x, center.y) radius:0 toCenter: NSMakePoint(center.x, center.y) radius: radius.width options:0];
			}

			virtual void MP_STDCALL SetCenter(GmpiDrawing_API::MP1_POINT pcenter) override
			{
				center = pcenter;
			}

			virtual void MP_STDCALL SetGradientOriginOffset(GmpiDrawing_API::MP1_POINT gradientOriginOffset) override
			{
			}

			virtual void MP_STDCALL SetRadiusX(float radiusX) override
			{
				radius.width = radiusX;
			}

			virtual void MP_STDCALL SetRadiusY(float radiusY) override
			{
				radius.height = radiusY;
			}
            
            virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory **factory) override
            {
                *factory = factory_;
            }
            
			GMPI_REFCOUNT;
			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_RADIALGRADIENTBRUSH_MPGUI, GmpiDrawing_API::IMpRadialGradientBrush);
		};

		class TextFormat : public CocoaWrapper<GmpiDrawing_API::IMpTextFormat, const __CFDictionary>
		{
//			std::wstring_convert<std::codecvt_utf8<wchar_t>>* stringConverter; // constructed once is much faster.
		public:
			std::string fontFamilyName;
			GmpiDrawing_API::MP1_FONT_WEIGHT fontWeight;
			GmpiDrawing_API::MP1_FONT_STYLE fontStyle;
			GmpiDrawing_API::MP1_FONT_STRETCH fontStretch;
			GmpiDrawing_API::MP1_TEXT_ALIGNMENT textAlignment;
			GmpiDrawing_API::MP1_PARAGRAPH_ALIGNMENT paragraphAlignment;
			float fontSize;

			static const char* fontSubstitute(const char* windowsFont)
			{
				static std::map< std::string, std::string > substitutes =
				{
					{"Arial", "Helvetica"},
					{"Arial Black", "Arial Black"},
					{"Comic Sans MS", "Comic Sans MS"},
					{"Courier New", "Courier New"},
					{"Georgia", "Georgia"},
					{"Impact", "Helvetica"},
					{"Tahoma", "Geneva"},
					{"MS Sans Serif4", "Geneva"},
					{"MS Serif", "New York"},
					{"Times New Roman", "Times"},
				};

				auto it = substitutes.find(windowsFont);
				if (it != substitutes.end())
					return (*it).second.c_str();

				return "Lucida Grande";
			}

			NSMutableDictionary* native2;

			TextFormat(std::wstring_convert<std::codecvt_utf8<wchar_t>>* pstringConverter, const char* pfontFamilyName, GmpiDrawing_API::MP1_FONT_WEIGHT pfontWeight, GmpiDrawing_API::MP1_FONT_STYLE pfontStyle, GmpiDrawing_API::MP1_FONT_STRETCH pfontStretch, float pfontSize) :
				CocoaWrapper<GmpiDrawing_API::IMpTextFormat, const __CFDictionary>(nullptr)
//				, stringConverter(pstringConverter)
				, fontWeight(pfontWeight)
				, fontStyle(pfontStyle)
				, fontStretch(pfontStretch)
				, fontSize(pfontSize)
			{
				fontFamilyName = fontSubstitute(pfontFamilyName);

//				auto nativefont = [NSFont fontWithName : [NSString stringWithCString : fontFamilyName.c_str() encoding : NSUTF8StringEncoding] size : fontSize];
                
                NSFontTraitMask fontTraits = 0;
                if(pfontWeight >= GmpiDrawing_API::MP1_FONT_WEIGHT_DEMI_BOLD)
                    fontTraits |= NSBoldFontMask;
                
                if(pfontStyle == GmpiDrawing_API::MP1_FONT_STYLE_ITALIC)
                    fontTraits |= NSItalicFontMask;
                
                NSFontManager* fontManager = [NSFontManager sharedFontManager];
                NSFont* nativefont = [fontManager fontWithFamily:[NSString stringWithCString: fontFamilyName.c_str() encoding: NSUTF8StringEncoding ] traits:fontTraits weight:5 size:fontSize ];

				NSMutableParagraphStyle* style = [[NSMutableParagraphStyle alloc] init];
				[style setAlignment : NSTextAlignmentLeft];

				native2 = [[NSMutableDictionary alloc] initWithObjectsAndKeys:
				nativefont, NSFontAttributeName,
					style, NSParagraphStyleAttributeName,
					nil];
			}

			virtual int32_t MP_STDCALL SetTextAlignment(GmpiDrawing_API::MP1_TEXT_ALIGNMENT ptextAlignment) override
			{
				textAlignment = ptextAlignment;

				switch (textAlignment)
				{
				case (int)GmpiDrawing::TextAlignment::Leading: // Left.
					[native2[NSParagraphStyleAttributeName] setAlignment:NSTextAlignmentLeft];
					break;
				case (int)GmpiDrawing::TextAlignment::Trailing: // Right
					[native2[NSParagraphStyleAttributeName] setAlignment:NSTextAlignmentRight];
					break;
				case (int)GmpiDrawing::TextAlignment::Center:
					[native2[NSParagraphStyleAttributeName] setAlignment:NSTextAlignmentCenter];
					break;
				}

				return gmpi::MP_OK;
			}

			virtual int32_t MP_STDCALL SetParagraphAlignment(GmpiDrawing_API::MP1_PARAGRAPH_ALIGNMENT pparagraphAlignment) override
			{
				paragraphAlignment = pparagraphAlignment;
				return gmpi::MP_OK;
			}

			virtual int32_t MP_STDCALL SetWordWrapping(GmpiDrawing_API::MP1_WORD_WRAPPING wordWrapping) override
			{
				return gmpi::MP_NOSUPPORT;
			}

			virtual int32_t MP_STDCALL SetLineSpacing(float lineSpacing, float baseline) override
			{
				return gmpi::MP_NOSUPPORT;
			}

			virtual int32_t MP_STDCALL GetFontMetrics(GmpiDrawing_API::MP1_FONT_METRICS* returnFontMetrics) override
			{
				returnFontMetrics->xHeight = [native2[NSFontAttributeName] xHeight];
				returnFontMetrics->ascent = [native2[NSFontAttributeName] ascender];
				returnFontMetrics->descent = -[native2[NSFontAttributeName] descender]; // Descent is negative on OSX (positive on Windows)
				returnFontMetrics->lineGap = [native2[NSFontAttributeName] leading];
				returnFontMetrics->capHeight = [native2[NSFontAttributeName] capHeight];
				returnFontMetrics->underlinePosition = [native2[NSFontAttributeName] underlinePosition];
				returnFontMetrics->underlineThickness = [native2[NSFontAttributeName] underlineThickness];
				returnFontMetrics->strikethroughPosition = returnFontMetrics->xHeight / 2;
				returnFontMetrics->strikethroughThickness = returnFontMetrics->underlineThickness;

				return gmpi::MP_OK;
			}

			// TODO!!!: Probly needs to accept constraint rect like DirectWrite. !!!
			//	virtual void MP_STDCALL GetTextExtentU(const char* utf8String, int32_t stringLength, GmpiDrawing::Size& returnSize)
			virtual void MP_STDCALL GetTextExtentU(const char* utf8String, int32_t stringLength, GmpiDrawing_API::MP1_SIZE* returnSize) override
			{
				auto str = [NSString stringWithCString : utf8String encoding : NSUTF8StringEncoding];

				auto r = [str sizeWithAttributes : native2];
				returnSize->width = r.width;
				returnSize->height = r.height;
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_TEXTFORMAT_MPGUI, GmpiDrawing_API::IMpTextFormat);
			GMPI_REFCOUNT;
		};


		class bitmapPixels : public GmpiDrawing_API::IMpBitmapPixels
		{
			//	bool alphaPremultiplied;
			int bytesPerRow;
			std::vector<int32_t> pixels;
            NSImageRep* bitmapPixels_;
            NSImage* inBitmap_;
			NSBitmapImageRep * bitmap2;
			int32_t flags;
            
        public:
            bitmapPixels(NSImage* inBitmap, bool _alphaPremultiplied, int32_t pflags) :
				flags(pflags)
			{
                inBitmap_ = inBitmap;
				bitmapPixels_ = nil; // [[inBitmap representations] objectAtIndex:0];

				{
					// Get largest bitmap representation. (most detail).
					for (NSImageRep* imagerep in[inBitmap_ representations])
					{
						if ([imagerep isKindOfClass : [NSBitmapImageRep class]])
						{
							if (bitmapPixels_ == nil) {
								bitmapPixels_ = imagerep;
							}
							if ([bitmapPixels_ pixelsHigh]<[imagerep pixelsHigh]) {
								bitmapPixels_ = imagerep;
							}
						}
					}
				}

				NSSize s = [inBitmap size];
				bytesPerRow = s.width * 4;

				pixels.assign(s.width * s.height, 0);
				unsigned char* pixelDest = (unsigned char*)pixels.data();
                
				bitmap2 = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&pixelDest
					pixelsWide : s.width
					pixelsHigh : s.height
					bitsPerSample : 8
					samplesPerPixel : 4
					hasAlpha : YES
					isPlanar : NO
                           // The “Device” color-space names represent color spaces in which component values are applied to devices as specified. There is no optimization or adjustment for differences between devices in how they render colors.
					colorSpaceName : NSDeviceRGBColorSpace // NSCalibratedRGBColorSpace
                    bitmapFormat : 0
					bytesPerRow : bytesPerRow
					bitsPerPixel : 32];

				NSGraphicsContext * context;
				context = [NSGraphicsContext graphicsContextWithBitmapImageRep : bitmap2];
				[NSGraphicsContext saveGraphicsState];
				[NSGraphicsContext setCurrentContext : context];
//				[inBitmap drawInRect : NSMakeRect(0, 0, s.width, s.height)];
                
                // Mac stored images "upside down" compared to Windows.
                /*
                NSAffineTransform *t = [NSAffineTransform transform];
                [t translateXBy:0 yBy:s.height];
                [t scaleXBy:1 yBy:-1];
                [t concat];
*/
                [inBitmap drawAtPoint: NSZeroPoint fromRect: NSZeroRect operation: NSCompositeCopy fraction: 1.0];
				[NSGraphicsContext restoreGraphicsState];
			}

			~bitmapPixels()
			{
                // TODO, don't work. seems to blur it, really need to ber able to lock/discard (for hit testing) without wasting all this effort.
				if (0 != (flags & GmpiDrawing_API::MP1_BITMAP_LOCK_WRITE))
				{
                    NSImage *producedImage = [[NSImage alloc] init];
                    [producedImage addRepresentation:bitmap2];
//                    [bitmapPixels_ release];
                    
                    [inBitmap_ lockFocus];
                    //    [bitmap2 drawInRect: NSMakeRect(0,0,producedImage.size.width, producedImage.size.height)];
                    // fromRect:NSZeroRect => entire image is drawn.
                    [producedImage drawAtPoint: NSZeroPoint fromRect: NSZeroRect operation: NSCompositeCopy fraction: 1.0];
                    
                    [inBitmap_ unlockFocus];
                    
                    [bitmap2 release];
				}
			}

			virtual uint8_t* MP_STDCALL getAddress() const override { return (uint8_t*)pixels.data(); };
			virtual int32_t MP_STDCALL getBytesPerRow() const override { return bytesPerRow; };
			virtual int32_t MP_STDCALL getPixelFormat() const override { return kRGBA; };

			inline uint8_t fast8bitScale(uint8_t a, uint8_t b)
			{
				int t = (int)a * (int)b;
				return (uint8_t)((t + 1 + (t >> 8)) >> 8); // fast way to divide by 255
			}

			void premultiplyAlpha()
			{
				for (auto& p : pixels)
				{
					auto pixel = reinterpret_cast<uint8_t*>(&p);
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
				}
			}

			//-----------------------------------------------------------------------------
			void unpremultiplyAlpha()
			{
				for (auto& p : pixels)
				{
					auto pixel = reinterpret_cast<uint8_t*>(&p);
					if (pixel[3] == 0)
					{
						pixel[0] = (uint32_t)(pixel[0] * 255) / pixel[3];
						pixel[1] = (uint32_t)(pixel[1] * 255) / pixel[3];
						pixel[2] = (uint32_t)(pixel[2] * 255) / pixel[3];
					}
				}
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_BITMAP_PIXELS_MPGUI, GmpiDrawing_API::IMpBitmapPixels);
			GMPI_REFCOUNT;
		};

		class Bitmap : public GmpiDrawing_API::IMpBitmap
		{
		public:
			NSImage* nativeBitmap_;

			Bitmap(const char* utf8Uri) :
				nativeBitmap_(nullptr)
			{
#if 1
				NSString * url = [NSString stringWithCString : utf8Uri encoding : NSUTF8StringEncoding];
				nativeBitmap_ = [[NSImage alloc] initWithContentsOfFile:url];
//				[nativeBitmap_ setFlipped : TRUE]; // cache bitmap up correct way (else OSX flips it).
                
                // undo scaling of image (which reports scaled size, screwing up animation frame).
                NSSize max = NSZeroSize;
                for (NSObject* o in nativeBitmap_.representations) {
                    if ([o isKindOfClass: NSImageRep.class]) {
                        NSImageRep* r = (NSImageRep*)o;
                        if (r.pixelsWide != NSImageRepMatchesDevice && r.pixelsHigh != NSImageRepMatchesDevice) {
                            max.width = MAX(max.width, r.pixelsWide);
                            max.height = MAX(max.height, r.pixelsHigh);
                        }
                    }
                }
                if (max.width > 0 && max.height > 0) {
                    nativeBitmap_.size = max;
                }
                
#else
                // On windows bitmaps are stored the other way up.
                // For direct memory access functions to work correctly, we
                // have to flip the image unnaturally (for mac). We also have to draw it upside down later to compensate.
                NSString * url = [NSString stringWithCString : utf8Uri encoding : NSUTF8StringEncoding];
                auto inputImage = [[NSImage alloc] initWithContentsOfFile:url];

                NSSize dimensions = [inputImage size];
                
                nativeBitmap_ = [[NSImage alloc] initWithSize:dimensions];
                [nativeBitmap_ lockFocus];
                
                NSAffineTransform *t = [NSAffineTransform transform];
                [t translateXBy:dimensions.width yBy:dimensions.height];
                [t scaleXBy:-1 yBy:-1];
                [t concat];
                
                [inputImage drawAtPoint:NSMakePoint(0,0)
                               fromRect:NSMakeRect(0, 0, dimensions.width, dimensions.height)
                              operation:NSCompositeCopy fraction:1.0];
                [nativeBitmap_ unlockFocus];
               
#endif
			}
            
            Bitmap(int32_t width, int32_t height)
            {
                nativeBitmap_ = [[NSImage alloc] initWithSize:NSMakeSize((CGFloat)width, (CGFloat)height)];
                [nativeBitmap_ retain];
            }
            
			Bitmap(NSImage* native) : nativeBitmap_(native)
			{
				[nativeBitmap_ retain];
			}

			bool isLoaded()
			{
				return nativeBitmap_ != nil;
			}

			virtual ~Bitmap()
			{
				[nativeBitmap_ release];
			}

			inline NSImage* GetNativeBitmap()
			{
				return nativeBitmap_;
			}

			virtual GmpiDrawing_API::MP1_SIZE MP_STDCALL GetSizeF() override
			{
				NSSize s = [nativeBitmap_ size];
				return GmpiDrawing::Size(s.width, s.height);
			}

			virtual int32_t MP_STDCALL GetSize(GmpiDrawing_API::MP1_SIZE_U* returnSize) override
			{
				NSSize s = [nativeBitmap_ size];

				returnSize->width = FastRealToIntTruncateTowardZero(0.5f + s.width);
				returnSize->height = FastRealToIntTruncateTowardZero(0.5f + s.height);

				/* hmm, assumes image representation at index 0 is actual size. size is already set correctly in constructor.
				 * 
#ifdef _DEBUG
                // Check images NOT scaled by cocoa.
                // https://stackoverflow.com/questions/2190027/nsimage-acting-weird
                NSImageRep *rep = [[nativeBitmap_ representations] objectAtIndex:0];
                
                assert(returnSize->width == rep.pixelsWide);
                assert(returnSize->height == rep.pixelsHigh);
#endif
				 */
				return gmpi::MP_OK;
			}

			virtual int32_t MP_STDCALL lockPixelsOld(GmpiDrawing_API::IMpBitmapPixels** returnInterface, bool alphaPremultiplied) override
			{
				*returnInterface = 0;

				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new bitmapPixels(nativeBitmap_, alphaPremultiplied, GmpiDrawing_API::MP1_BITMAP_LOCK_READ | GmpiDrawing_API::MP1_BITMAP_LOCK_WRITE));

				return b2->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_PIXELS_MPGUI, (void**)(returnInterface));
			}

			virtual int32_t MP_STDCALL lockPixels(GmpiDrawing_API::IMpBitmapPixels** returnInterface, int32_t flags) override
			{
				*returnInterface = 0;

				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new bitmapPixels(nativeBitmap_, true, flags));

				return b2->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_PIXELS_MPGUI, (void**)(returnInterface));
			}

			virtual void MP_STDCALL ApplyAlphaCorrection() override {};

			void ApplyAlphaCorrection2() {};

			virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory** factory) override
			{
				assert(false);
				//		native_->GetFactory((ID2D1Factory**)factory);
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_BITMAP_MPGUI, GmpiDrawing_API::IMpBitmap);
			GMPI_REFCOUNT;
		};


		class GeometrySink : public gmpi::generic_graphics::GeometrySink
		{
			NSBezierPath* geometry_;

		public:
			GeometrySink(NSBezierPath* geometry) : geometry_(geometry) {}

			virtual void MP_STDCALL SetFillMode(GmpiDrawing_API::MP1_FILL_MODE fillMode)
			{
				switch (fillMode)
				{
				case GmpiDrawing_API::MP1_FILL_MODE_ALTERNATE:
					[geometry_ setWindingRule : NSEvenOddWindingRule];
					break;

				case GmpiDrawing_API::MP1_FILL_MODE_WINDING:
					[geometry_ setWindingRule : NSNonZeroWindingRule];
					break;
				case GmpiDrawing_API::MP1_GAMMA_FORCE_DWORD:
					break;
				}
			}
			virtual void MP_STDCALL SetSegmentFlags(GmpiDrawing_API::MP1_PATH_SEGMENT vertexFlags)
			{
				//		geometrysink_->SetSegmentFlags((D2D1_PATH_SEGMENT)vertexFlags);
			}
			virtual void MP_STDCALL BeginFigure(GmpiDrawing_API::MP1_POINT startPoint, GmpiDrawing_API::MP1_FIGURE_BEGIN figureBegin) override
			{
				[geometry_ moveToPoint : NSMakePoint(startPoint.x, startPoint.y)];

				lastPoint = startPoint;
			}

			virtual void MP_STDCALL EndFigure(GmpiDrawing_API::MP1_FIGURE_END figureEnd) override
			{
				if (figureEnd == GmpiDrawing_API::MP1_FIGURE_END_CLOSED)
				{
					[geometry_ closePath];
				}
			}

			virtual void MP_STDCALL AddLine(GmpiDrawing_API::MP1_POINT point) override
			{
				[geometry_ lineToPoint : NSMakePoint(point.x, point.y)];

				lastPoint = point;
			}

			virtual void MP_STDCALL AddBezier(const GmpiDrawing_API::MP1_BEZIER_SEGMENT* bezier) override
			{
				[geometry_ curveToPoint : NSMakePoint(bezier->point3.x, bezier->point3.y)
					controlPoint1 : NSMakePoint(bezier->point1.x, bezier->point1.y)
					controlPoint2 : NSMakePoint(bezier->point2.x, bezier->point2.y)];

				lastPoint = bezier->point3;
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_GEOMETRYSINK_MPGUI, GmpiDrawing_API::IMpGeometrySink);
			GMPI_REFCOUNT;
		};


		class PathGeometry : public GmpiDrawing_API::IMpPathGeometry
		{
			NSBezierPath* geometry_;

		public:
			PathGeometry()
			{
				geometry_ = [NSBezierPath bezierPath];
                [geometry_ retain]; // TODO: Why only NSBezierPath needs manual retain??? !!!
			}

			~PathGeometry()
			{
				//	auto release pool handles it?
                [geometry_ release];
			}

			inline NSBezierPath* native()
			{
				return geometry_;
			}

			virtual int32_t MP_STDCALL Open(GmpiDrawing_API::IMpGeometrySink** geometrySink) override
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new GeometrySink(geometry_));

				return b2->queryInterface(GmpiDrawing_API::SE_IID_GEOMETRYSINK_MPGUI, reinterpret_cast<void**>(geometrySink));
			}

			virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory** factory) override
			{
				//		native_->GetFactory((ID2D1Factory**)factory);
			}
            
            CGMutablePathRef getCGPath() // could be cached.
            {
                CGMutablePathRef cgPath = CGPathCreateMutable();
                NSInteger n = [geometry_ elementCount];
                
                for (NSInteger i = 0; i < n; i++) {
                    NSPoint ps[3];
                    switch ([geometry_ elementAtIndex:i associatedPoints:ps]) {
                        case NSMoveToBezierPathElement: {
                            CGPathMoveToPoint(cgPath, NULL, ps[0].x, ps[0].y);
                            break;
                        }
                        case NSLineToBezierPathElement: {
                            CGPathAddLineToPoint(cgPath, NULL, ps[0].x, ps[0].y);
                            break;
                        }
                        case NSCurveToBezierPathElement: {
                            CGPathAddCurveToPoint(cgPath, NULL, ps[0].x, ps[0].y, ps[1].x, ps[1].y, ps[2].x, ps[2].y);
                            break;
                        }
                        case NSClosePathBezierPathElement: {
                            CGPathCloseSubpath(cgPath);
                            break;
                        }
                        default:
                            assert(false && @"Invalid NSBezierPathElement");
                    }
                }
                return cgPath;
            }

			virtual int32_t MP_STDCALL StrokeContainsPoint(GmpiDrawing_API::MP1_POINT point, float strokeWidth, GmpiDrawing_API::IMpStrokeStyle* strokeStyle, const GmpiDrawing_API::MP1_MATRIX_3X2* worldTransform, bool* returnContains) override
			{
                auto cgPath2 = getCGPath();
                
                CGPathRef hitTargetPath = CGPathCreateCopyByStrokingPath(cgPath2, NULL, (CGFloat) strokeWidth, (CGLineCap) [geometry_ lineCapStyle], (CGLineJoin)[geometry_ lineJoinStyle], [geometry_ miterLimit] );
                
                CGPathRelease(cgPath2);
                
                CGPoint cgpoint = CGPointMake(point.x, point.y);
                *returnContains = (bool) CGPathContainsPoint(hitTargetPath, NULL, cgpoint, (bool)[geometry_ windingRule]);
                
                CGPathRelease(hitTargetPath);
				return gmpi::MP_OK;
			}
            
			virtual int32_t MP_STDCALL FillContainsPoint(GmpiDrawing_API::MP1_POINT point, const GmpiDrawing_API::MP1_MATRIX_3X2* worldTransform, bool* returnContains) override
			{
                *returnContains = [geometry_ containsPoint:NSMakePoint(point.x, point.y)];
				return gmpi::MP_OK;
			}
            
			virtual int32_t MP_STDCALL GetWidenedBounds(float strokeWidth, GmpiDrawing_API::IMpStrokeStyle* strokeStyle, const GmpiDrawing_API::MP1_MATRIX_3X2* worldTransform, GmpiDrawing_API::MP1_RECT* returnBounds) override
			{
                const float radius = ceilf(strokeWidth * 0.5f);
                auto nativeRect = [geometry_ bounds];
                returnBounds->left = nativeRect.origin.x - radius;
                returnBounds->top = nativeRect.origin.y - radius;
                returnBounds->right = nativeRect.origin.x + nativeRect.size.width + radius;
                returnBounds->bottom = nativeRect.origin.y + nativeRect.size.height + radius;

				return gmpi::MP_OK;
			}
			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_PATHGEOMETRY_MPGUI, GmpiDrawing_API::IMpPathGeometry);
			GMPI_REFCOUNT;
		};

		class DrawingFactory : public GmpiDrawing_API::IMpFactory
		{
		public:
			std::wstring_convert<std::codecvt_utf8<wchar_t>> stringConverter; // cached, as constructor is super-slow.

			DrawingFactory() {};

			virtual int32_t MP_STDCALL CreatePathGeometry(GmpiDrawing_API::IMpPathGeometry** pathGeometry) override
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new PathGeometry());

				return b2->queryInterface(GmpiDrawing_API::SE_IID_PATHGEOMETRY_MPGUI, reinterpret_cast<void **>(pathGeometry));
			}

			virtual int32_t MP_STDCALL CreateTextFormat(const char* fontFamilyName, void* unused /* fontCollection */, GmpiDrawing_API::MP1_FONT_WEIGHT fontWeight, GmpiDrawing_API::MP1_FONT_STYLE fontStyle, GmpiDrawing_API::MP1_FONT_STRETCH fontStretch, float fontSize, void* unused2 /* localeName */, GmpiDrawing_API::IMpTextFormat** textFormat) override
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new TextFormat(&stringConverter, fontFamilyName, fontWeight, fontStyle, fontStretch, fontSize));

				return b2->queryInterface(GmpiDrawing_API::SE_IID_TEXTFORMAT_MPGUI, reinterpret_cast<void **>(textFormat));
			}

			virtual int32_t MP_STDCALL CreateImage(int32_t width, int32_t height, GmpiDrawing_API::IMpBitmap** returnDiBitmap) override
			{
                *returnDiBitmap = nullptr;
                
                auto bm = new Bitmap(width, height);
                return bm->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_MPGUI, (void**)returnDiBitmap);
			}

			virtual int32_t MP_STDCALL LoadImageU(const char* utf8Uri, GmpiDrawing_API::IMpBitmap** returnDiBitmap) override
			{
				*returnDiBitmap = nullptr;

				auto bm = new Bitmap(utf8Uri);
				if (bm->isLoaded())
				{
					bm->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_MPGUI, (void**)returnDiBitmap);
					return gmpi::MP_OK;
				}

				delete bm;
				return gmpi::MP_FAIL;
			}

			virtual int32_t CreateStrokeStyle(const GmpiDrawing_API::MP1_STROKE_STYLE_PROPERTIES* strokeStyleProperties, float* dashes, int32_t dashesCount, GmpiDrawing_API::IMpStrokeStyle** returnValue) override
			{
				*returnValue = nullptr;

				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new gmpi::generic_graphics::StrokeStyle(this, strokeStyleProperties, dashes, dashesCount));

				return b2->queryInterface(GmpiDrawing_API::SE_IID_STROKESTYLE_MPGUI, reinterpret_cast<void **>(returnValue));
			}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_FACTORY_MPGUI, GmpiDrawing_API::IMpFactory);
			GMPI_REFCOUNT_NO_DELETE;
		};


		class GraphicsContext : public GmpiDrawing_API::IMpDeviceContext
		{
		protected:
			std::wstring_convert<std::codecvt_utf8<wchar_t>>* stringConverter; // cached, as constructor is super-slow.
			GmpiDrawing_API::IMpFactory* factory;
			std::vector<GmpiDrawing_API::MP1_RECT> clipRectStack;
			NSAffineTransform* currentTransform;
			NSView* view_;
            
		public:

			GraphicsContext(NSView* pview, GmpiDrawing_API::IMpFactory* pfactory) :
				factory(pfactory)
				, view_(pview)
			{
				currentTransform = [NSAffineTransform transform];
				/*
				context_->AddRef();

				const float defaultClipBounds = 100000.0f;
				GmpiDrawing_API::MP1_RECT r;
				r.top = r.left = -defaultClipBounds;
				r.bottom = r.right = defaultClipBounds;
				clipRectStack.push_back(r);

				stringConverter = &(dynamic_cast<DX_Factory*>(pfactory)->stringConverter);
				 */
			}

			~GraphicsContext()
			{
				//?        [currentTransform release];
			}

			virtual void MP_STDCALL GetFactory(GmpiDrawing_API::IMpFactory** pfactory) override
			{
				*pfactory = factory;
			}

			virtual void MP_STDCALL DrawRectangle(const GmpiDrawing_API::MP1_RECT* rect, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle) override
			{
                /*
				auto scb = dynamic_cast<const SolidColorBrush*>(brush);
				if (scb)
				{
					[scb->nativeColor() set];
				}
				NSBezierPath *bp = [NSBezierPath bezierPathWithRect : gmpi::cocoa::NSRectFromRect(*rect)];
				[bp stroke];
                */
                
                NSBezierPath* path = [NSBezierPath bezierPathWithRect : gmpi::cocoa::NSRectFromRect(*rect)];
                auto cocoabrush = dynamic_cast<const CocoaBrushBase*>(brush);
                if (cocoabrush)
                {
                    cocoabrush->StrokePath(path, strokeWidth, strokeStyle);
                }
			}

			virtual void MP_STDCALL FillRectangle(const GmpiDrawing_API::MP1_RECT* rect, const GmpiDrawing_API::IMpBrush* brush) override
			{
				NSBezierPath* rectPath = [NSBezierPath bezierPathWithRect : gmpi::cocoa::NSRectFromRect(*rect)];

				auto cocoabrush = dynamic_cast<const CocoaBrushBase*>(brush);
				if (cocoabrush)
				{
					cocoabrush->FillPath(rectPath);
				}
			}

			virtual void MP_STDCALL Clear(const GmpiDrawing_API::MP1_COLOR* clearColor) override
			{
				//		context_->Clear((D2D1_COLOR_F*)clearColor);
			}

			virtual void MP_STDCALL DrawLine(GmpiDrawing_API::MP1_POINT point0, GmpiDrawing_API::MP1_POINT point1, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle) override
			{
                /*
				auto scb = dynamic_cast<const SolidColorBrush*>(brush);
				if (scb)
				{
					[scb->nativeColor() set];
				}

				NSBezierPath* line = [NSBezierPath bezierPath];
				[line moveToPoint : NSMakePoint(point0.x, point0.y)];
				[line lineToPoint : NSMakePoint(point1.x, point1.y)];
				[line setLineWidth : strokeWidth];
                
                SetNativePenStrokeStyle(line, (GmpiDrawing_API::IMpStrokeStyle*) strokeStyle);
                
				[line stroke];
                */
                
                NSBezierPath* path = [NSBezierPath bezierPath];
                [path moveToPoint : NSMakePoint(point0.x, point0.y)];
                [path lineToPoint : NSMakePoint(point1.x, point1.y)];

                auto cocoabrush = dynamic_cast<const CocoaBrushBase*>(brush);
                if (cocoabrush)
                {
                    cocoabrush->StrokePath(path, strokeWidth, strokeStyle);
                }
			}

			virtual void MP_STDCALL DrawGeometry(const GmpiDrawing_API::IMpPathGeometry* geometry, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth = 1.0f, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle = 0) override
			{
				auto nsPath = ((PathGeometry*)geometry)->native();

				auto cocoabrush = dynamic_cast<const CocoaBrushBase*>(brush);
				if (cocoabrush)
				{
					cocoabrush->StrokePath(nsPath, strokeWidth, strokeStyle);
				}
			}

			virtual void MP_STDCALL FillGeometry(const GmpiDrawing_API::IMpPathGeometry* geometry, const GmpiDrawing_API::IMpBrush* brush, const GmpiDrawing_API::IMpBrush* opacityBrush) override
			{
				auto nsPath = ((PathGeometry*)geometry)->native();

				auto cocoabrush = dynamic_cast<const CocoaBrushBase*>(brush);
				if (cocoabrush)
				{
					cocoabrush->FillPath(nsPath);
				}
			}

			//	virtual void MP_STDCALL DrawTextU(const char* utf8String, int32_t stringLength, GmpiDrawing_API::IMpTextFormat* textFormat, GmpiDrawing::Rect rect, GmpiDrawing_API::IMpBrush* brush, int32_t flags = 0);
			virtual void MP_STDCALL DrawTextU(const char* utf8String, int32_t stringLength, const GmpiDrawing_API::IMpTextFormat* textFormat, const GmpiDrawing_API::MP1_RECT* layoutRect, const GmpiDrawing_API::IMpBrush* brush, int32_t flags) override
			{
				auto textformat = reinterpret_cast<const TextFormat*>(textFormat);

				auto scb = dynamic_cast<const SolidColorBrush*>(brush);
				if (scb)
				{
					[scb->nativeColor() set];
				}

				CGRect bounds = CGRectMake(layoutRect->left, layoutRect->top, layoutRect->right - layoutRect->left, layoutRect->bottom - layoutRect->top);

				if (textformat->paragraphAlignment != (int)GmpiDrawing::TextAlignment::Leading)
				{
					GmpiDrawing::Size textSize;
					const_cast<GmpiDrawing_API::IMpTextFormat*>(textFormat)->GetTextExtentU(utf8String, (int32_t)strlen(utf8String), &textSize);

					// Vertical text alignment.
					switch (textformat->paragraphAlignment)
					{
					case (int)GmpiDrawing::TextAlignment::Trailing:    // Bottom
						bounds.origin.y += bounds.size.height - textSize.height;
						bounds.size.height = textSize.height;
						break;

					case (int)GmpiDrawing::TextAlignment::Center:
						bounds.origin.y += (bounds.size.height - textSize.height) / 2;
						bounds.size.height = textSize.height;
						break;

					default:
						break;
					}
				}

				NSString* str = [NSString stringWithCString : utf8String encoding : NSUTF8StringEncoding];
                
                [textformat->native2 setObject:scb->nativeColor() forKey:NSForegroundColorAttributeName];
 //               [textformat->native2 setObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];
//                [textformat->native2 setObject:[NSColor redColor] forKey:NSBackgroundColorAttributeName];

 //               [textformat->native2 setObject :[NSNumber numberWithInt:NSSingleUnderlineStyle] forKey:NSUnderlineStyleAttributeName ];
                
				[str drawInRect : bounds withAttributes : textformat->native2];
			}

			virtual void MP_STDCALL DrawBitmap(const GmpiDrawing_API::IMpBitmap* mpBitmap, const GmpiDrawing_API::MP1_RECT* destinationRectangle, float opacity, /* MP1_BITMAP_INTERPOLATION_MODE*/ int32_t interpolationMode, const GmpiDrawing_API::MP1_RECT* sourceRectangle) override
			{
				auto bm = ((Bitmap*)mpBitmap);
				auto bitmap = bm->GetNativeBitmap();
				if (bitmap)
				{
                    GmpiDrawing_API::MP1_SIZE_U imageSize;
                    bm->GetSize(&imageSize);
                    
                    GmpiDrawing::Rect sourceRectangleFlipped(*sourceRectangle);
                    
                    sourceRectangleFlipped.bottom = imageSize.height - sourceRectangle->top;
                    sourceRectangleFlipped.top = imageSize.height - sourceRectangle->bottom;
//                    sourceRectangleFlipped.left = imageSize.width - sourceRectangle->right;
//                    sourceRectangleFlipped.right = imageSize.width - sourceRectangle->left;
                   
                    // For binary compatibilit with Windows, bitmaps are stored upside down.
                    // Need to correct when rendering.
/*
                    [NSGraphicsContext saveGraphicsState];
                    
                    NSAffineTransform *t = [NSAffineTransform transform];
                    [t translateXBy:destinationRectangle->right yBy:destinationRectangle->bottom];
                    [t scaleXBy:-1 yBy:-1];
                    [t concat];
*/
					[bitmap drawInRect : gmpi::cocoa::NSRectFromRect(*destinationRectangle)  fromRect : gmpi::cocoa::NSRectFromRect(sourceRectangleFlipped) operation : NSCompositeSourceOver fraction : opacity respectFlipped : TRUE hints : nil];
                    
//                    [NSGraphicsContext restoreGraphicsState];
				}
			}

			virtual void MP_STDCALL SetTransform(const GmpiDrawing_API::MP1_MATRIX_3X2* transform) override
			{
				// Remove the transformations by applying the inverse transform.
				[currentTransform invert];
				[currentTransform concat];

				NSAffineTransformStruct
					transformStruct = [currentTransform transformStruct];

				transformStruct.m11 = transform->_11;
				transformStruct.m12 = transform->_12;
				transformStruct.m21 = transform->_21;
				transformStruct.m22 = transform->_22;
				transformStruct.tX = transform->_31;
				transformStruct.tY = transform->_32;

				[currentTransform setTransformStruct : transformStruct];

				[currentTransform concat];
			}

			virtual void MP_STDCALL GetTransform(GmpiDrawing_API::MP1_MATRIX_3X2* transform) override
			{
				NSAffineTransformStruct
					transformStruct = [currentTransform transformStruct];

				transform->_11 = transformStruct.m11;
				transform->_12 = transformStruct.m12;
				transform->_21 = transformStruct.m21;
				transform->_22 = transformStruct.m22;
				transform->_31 = transformStruct.tX;
				transform->_32 = transformStruct.tY;
			}

			virtual int32_t MP_STDCALL CreateSolidColorBrush(const GmpiDrawing_API::MP1_COLOR* color, GmpiDrawing_API::IMpSolidColorBrush **solidColorBrush) override
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new SolidColorBrush(color, factory));

				b2->queryInterface(GmpiDrawing_API::SE_IID_SOLIDCOLORBRUSH_MPGUI, reinterpret_cast<void **>(solidColorBrush));

				return gmpi::MP_OK;
			}

			virtual int32_t MP_STDCALL CreateGradientStopCollection(const GmpiDrawing_API::MP1_GRADIENT_STOP* gradientStops, uint32_t gradientStopsCount, /* GmpiDrawing_API::MP1_GAMMA colorInterpolationGamma, GmpiDrawing_API::MP1_EXTEND_MODE extendMode,*/ GmpiDrawing_API::IMpGradientStopCollection** gradientStopCollection) override
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new GradientStopCollection(factory, gradientStops, gradientStopsCount));

				b2->queryInterface(GmpiDrawing_API::SE_IID_GRADIENTSTOPCOLLECTION_MPGUI, reinterpret_cast<void **>(gradientStopCollection));

				return gmpi::MP_OK;
			}

			virtual int32_t MP_STDCALL CreateLinearGradientBrush(const GmpiDrawing_API::MP1_LINEAR_GRADIENT_BRUSH_PROPERTIES* linearGradientBrushProperties, const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties, const  GmpiDrawing_API::IMpGradientStopCollection* gradientStopCollection, GmpiDrawing_API::IMpLinearGradientBrush** linearGradientBrush) override
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new LinearGradientBrush(factory, linearGradientBrushProperties, brushProperties, gradientStopCollection));

				return b2->queryInterface(GmpiDrawing_API::SE_IID_LINEARGRADIENTBRUSH_MPGUI, reinterpret_cast<void **>(linearGradientBrush));
			}

			virtual int32_t MP_STDCALL CreateBitmapBrush(const GmpiDrawing_API::IMpBitmap* bitmap, const GmpiDrawing_API::MP1_BITMAP_BRUSH_PROPERTIES* bitmapBrushProperties, const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties, GmpiDrawing_API::IMpBitmapBrush** bitmapBrush) override
			{
				// Not supported for now.
				return gmpi::MP_FAIL;
			}

			virtual int32_t MP_STDCALL CreateRadialGradientBrush(const GmpiDrawing_API::MP1_RADIAL_GRADIENT_BRUSH_PROPERTIES* radialGradientBrushProperties, const GmpiDrawing_API::MP1_BRUSH_PROPERTIES* brushProperties, const GmpiDrawing_API::IMpGradientStopCollection* gradientStopCollection, GmpiDrawing_API::IMpRadialGradientBrush** radialGradientBrush) override
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
				b2.Attach(new RadialGradientBrush(factory, radialGradientBrushProperties, brushProperties, gradientStopCollection ));

				return b2->queryInterface(GmpiDrawing_API::SE_IID_RADIALGRADIENTBRUSH_MPGUI, reinterpret_cast<void **>(radialGradientBrush));
			}

			virtual int32_t MP_STDCALL CreateCompatibleRenderTarget(const GmpiDrawing_API::MP1_SIZE* desiredSize, GmpiDrawing_API::IMpBitmapRenderTarget** bitmapRenderTarget) override;

			virtual void MP_STDCALL DrawRoundedRectangle(const GmpiDrawing_API::MP1_ROUNDED_RECT* roundedRect, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle) override
			{
				NSRect r = gmpi::cocoa::NSRectFromRect(roundedRect->rect);
				NSBezierPath* path = [NSBezierPath bezierPathWithRoundedRect : r xRadius : roundedRect->radiusX yRadius : roundedRect->radiusY];

                auto cocoabrush = dynamic_cast<const CocoaBrushBase*>(brush);
                if (cocoabrush)
                {
                    cocoabrush->StrokePath(path, strokeWidth, strokeStyle);
                }
            }

			virtual void MP_STDCALL FillRoundedRectangle(const GmpiDrawing_API::MP1_ROUNDED_RECT* roundedRect, const GmpiDrawing_API::IMpBrush* brush) override
			{
				NSRect r = gmpi::cocoa::NSRectFromRect(roundedRect->rect);
				NSBezierPath* rectPath = [NSBezierPath bezierPathWithRoundedRect : r xRadius:roundedRect->radiusX yRadius: roundedRect->radiusY];

				auto cocoabrush = dynamic_cast<const CocoaBrushBase*>(brush);
				if (cocoabrush)
				{
					cocoabrush->FillPath(rectPath);
				}
			}

			virtual void MP_STDCALL DrawEllipse(const GmpiDrawing_API::MP1_ELLIPSE* ellipse, const GmpiDrawing_API::IMpBrush* brush, float strokeWidth, const GmpiDrawing_API::IMpStrokeStyle* strokeStyle) override
			{
				NSRect r = NSMakeRect(ellipse->point.x - ellipse->radiusX, ellipse->point.y - ellipse->radiusY, ellipse->radiusX * 2.0f, ellipse->radiusY * 2.0f);

				NSBezierPath* path = [NSBezierPath bezierPathWithOvalInRect : r];

				auto cocoabrush = dynamic_cast<const CocoaBrushBase*>(brush);
				if (cocoabrush)
				{
                    cocoabrush->StrokePath(path, strokeWidth, strokeStyle);
				}
			}

			virtual void MP_STDCALL FillEllipse(const GmpiDrawing_API::MP1_ELLIPSE* ellipse, const GmpiDrawing_API::IMpBrush* brush) override
			{
                NSRect r = NSMakeRect(ellipse->point.x - ellipse->radiusX, ellipse->point.y - ellipse->radiusY, ellipse->radiusX * 2.0f, ellipse->radiusY * 2.0f);

				NSBezierPath* rectPath = [NSBezierPath bezierPathWithOvalInRect : r];

				auto cocoabrush = dynamic_cast<const CocoaBrushBase*>(brush);
				if (cocoabrush)
				{
					cocoabrush->FillPath(rectPath);
				}
			}

			virtual void MP_STDCALL PushAxisAlignedClip(const GmpiDrawing_API::MP1_RECT* clipRect/*, GmpiDrawing_API::MP1_ANTIALIAS_MODE antialiasMode*/) override
			{
				clipRectStack.push_back(*clipRect);

				// Save the current clipping region
				[NSGraphicsContext saveGraphicsState];

				NSRectClip(NSRectFromRect(*clipRect));
			}

			virtual void MP_STDCALL PopAxisAlignedClip() override
			{
				clipRectStack.pop_back();

				// Restore the clipping region for further drawing
				[NSGraphicsContext restoreGraphicsState];
			}

			virtual void MP_STDCALL GetAxisAlignedClip(GmpiDrawing_API::MP1_RECT* returnClipRect) override
			{
                GmpiDrawing::Matrix3x2 currentTransform;
                GetTransform(&currentTransform);
                currentTransform.Invert();
				*returnClipRect = currentTransform.TransformRect(clipRectStack.back());
			}

			virtual void MP_STDCALL BeginDraw() override
			{
				//		context_->BeginDraw();
			}

			virtual int32_t MP_STDCALL EndDraw() override
			{
				//		auto hr = context_->EndDraw();

				//		return hr == S_OK ? (gmpi::MP_OK) : (gmpi::MP_FAIL);
				return gmpi::MP_OK;
			}
/*
            virtual int32_t MP_STDCALL CreateMesh(GmpiDrawing_API::IMpMesh** returnObject) override
            {
                *returnObject = nullptr;
                return gmpi::MP_FAIL;
            }
            
            virtual void MP_STDCALL FillMesh(const GmpiDrawing_API::IMpMesh* mesh, const GmpiDrawing_API::IMpBrush* brush) override
            {
                
            }
*/
			//	virtual void MP_STDCALL InsetNewMethodHere(){}

			GMPI_QUERYINTERFACE1(GmpiDrawing_API::SE_IID_DEVICECONTEXT_MPGUI, GmpiDrawing_API::IMpDeviceContext);
			GMPI_REFCOUNT_NO_DELETE;
		};

		// https://stackoverflow.com/questions/10627557/mac-os-x-drawing-into-an-offscreen-nsgraphicscontext-using-cgcontextref-c-funct
		class bitmapRenderTarget : public GraphicsContext // emulated by carefull layout public GmpiDrawing_API::IMpBitmapRenderTarget
		{
			NSImage* image;

		public:
			bitmapRenderTarget(GmpiDrawing_API::IMpFactory* pfactory, const GmpiDrawing_API::MP1_SIZE* desiredSize) :
				GraphicsContext(nullptr, pfactory)
			{
				NSRect r = NSMakeRect(0.0, 0.0, desiredSize->width, desiredSize->height);
                image = [[NSImage alloc] initWithSize:r.size];
//                [image setFlipped:TRUE];
			}

            virtual void MP_STDCALL BeginDraw() override
            {
                // To match Flipped View, Flip Bitmap Context too.
                // (Alternative is [image setFlipped:TRUE] in contructor, but that method is deprected).
                [image lockFocusFlipped:TRUE];
//                [image lockFocus];
/*
                [NSGraphicsContext saveGraphicsState];
                
                NSAffineTransform *t = [NSAffineTransform transform];
                [t translateXBy:0 yBy:image.size.height];
                [t scaleXBy:1 yBy:-1];
                [t concat];
  */
                GraphicsContext::BeginDraw();
            }
            
            virtual int32_t MP_STDCALL EndDraw() override
            {
                auto r = GraphicsContext::EndDraw();
                [image unlockFocus];

//                [NSGraphicsContext restoreGraphicsState];
                
                return r;
            }
            
			~bitmapRenderTarget()
			{
				[image release];
			}

            // MUST BE FIRST VIRTUAL FUNCTION!
			virtual int32_t MP_STDCALL GetBitmap(GmpiDrawing_API::IMpBitmap** returnBitmap)
			{
				gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b;
				b.Attach(new Bitmap(image));
				return b->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_MPGUI, reinterpret_cast<void**>(returnBitmap));
			}

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

		int32_t MP_STDCALL GraphicsContext::CreateCompatibleRenderTarget(const GmpiDrawing_API::MP1_SIZE* desiredSize, GmpiDrawing_API::IMpBitmapRenderTarget** bitmapRenderTarget)
		{
			gmpi_sdk::mp_shared_ptr<gmpi::IMpUnknown> b2;
			b2.Attach(new class bitmapRenderTarget(factory, desiredSize));

			return b2->queryInterface(GmpiDrawing_API::SE_IID_BITMAP_RENDERTARGET_MPGUI, reinterpret_cast<void **>(bitmapRenderTarget));
		}

	} // namespace
} // namespace
