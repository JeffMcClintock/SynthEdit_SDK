#pragma once
#ifndef GMPI_DRAWING_API_H_INCLUDED
#define GMPI_DRAWING_API_H_INCLUDED

/*
#include "Drawing_API.h"
using namespace GmpiDrawing_API;
*/

// todo, no dependancy on Gmpi.
#include "mp_sdk_common.h"

namespace GmpiDrawing_API
{
	enum MP1_FONT_WEIGHT
	{
		MP1_FONT_WEIGHT_THIN = 100

		,MP1_FONT_WEIGHT_EXTRA_LIGHT = 200

		,MP1_FONT_WEIGHT_ULTRA_LIGHT = 200

		,MP1_FONT_WEIGHT_LIGHT = 300

		,MP1_FONT_WEIGHT_NORMAL = 400

		,MP1_FONT_WEIGHT_REGULAR = 400

		,MP1_FONT_WEIGHT_MEDIUM = 500

		,MP1_FONT_WEIGHT_DEMI_BOLD = 600

		,MP1_FONT_WEIGHT_SEMI_BOLD = 600

		,MP1_FONT_WEIGHT_BOLD = 700

		,MP1_FONT_WEIGHT_EXTRA_BOLD = 800

		,MP1_FONT_WEIGHT_ULTRA_BOLD = 800

		,MP1_FONT_WEIGHT_BLACK = 900

		,MP1_FONT_WEIGHT_HEAVY = 900

		,MP1_FONT_WEIGHT_EXTRA_BLACK = 950

		,MP1_FONT_WEIGHT_ULTRA_BLACK = 950
	};

	enum MP1_FONT_STRETCH
	{
		MP1_FONT_STRETCH_UNDEFINED = 0

		,MP1_FONT_STRETCH_ULTRA_CONDENSED = 1

		,MP1_FONT_STRETCH_EXTRA_CONDENSED = 2

		,MP1_FONT_STRETCH_CONDENSED = 3

		,MP1_FONT_STRETCH_SEMI_CONDENSED = 4

		,MP1_FONT_STRETCH_NORMAL = 5

		,MP1_FONT_STRETCH_MEDIUM = 5

		,MP1_FONT_STRETCH_SEMI_EXPANDED = 6

		,MP1_FONT_STRETCH_EXPANDED = 7

		,MP1_FONT_STRETCH_EXTRA_EXPANDED = 8

		,MP1_FONT_STRETCH_ULTRA_EXPANDED = 9
	};

	enum MP1_FONT_STYLE
	{
		MP1_FONT_STYLE_NORMAL = 0

		,MP1_FONT_STYLE_OBLIQUE = 1

		,MP1_FONT_STYLE_ITALIC = 2
	};

	enum MP1_TEXT_ALIGNMENT
	{
		MP1_TEXT_ALIGNMENT_LEADING = 0

		,MP1_TEXT_ALIGNMENT_TRAILING = 1

		,MP1_TEXT_ALIGNMENT_CENTER = 2
	};

	enum MP1_PARAGRAPH_ALIGNMENT
	{
		MP1_PARAGRAPH_ALIGNMENT_NEAR = 0

		,MP1_PARAGRAPH_ALIGNMENT_FAR = 1

		,MP1_PARAGRAPH_ALIGNMENT_CENTER = 2
	};

	enum MP1_WORD_WRAPPING
	{
		MP1_WORD_WRAPPING_WRAP = 0

		,MP1_WORD_WRAPPING_NO_WRAP = 1
	};

	enum MP1_ALPHA_MODE
	{
		MP1_ALPHA_MODE_UNKNOWN = 0

		,MP1_ALPHA_MODE_PREMULTIPLIED = 1

		,MP1_ALPHA_MODE_STRAIGHT = 2

		,MP1_ALPHA_MODE_IGNORE = 3

		,MP1_ALPHA_MODE_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_BITMAP_LOCK_FLAGS
	{
		MP1_BITMAP_LOCK_READ = 0x1,
		MP1_BITMAP_LOCK_WRITE = 0x2,
		MP1_BITMAP_LOCK_FLAGS_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_GAMMA
	{
		MP1_GAMMA_2_2 = 0

		,MP1_GAMMA_1_0 = 1

		,MP1_GAMMA_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_OPACITY_MASK_CONTENT
	{
		MP1_OPACITY_MASK_CONTENT_GRAPHICS = 0

		,MP1_OPACITY_MASK_CONTENT_TEXT_NATURAL = 1

		,MP1_OPACITY_MASK_CONTENT_TEXT_GDI_COMPATIBLE = 2

		,MP1_OPACITY_MASK_CONTENT_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_EXTEND_MODE
	{
		MP1_EXTEND_MODE_CLAMP = 0

		,MP1_EXTEND_MODE_WRAP = 1

		,MP1_EXTEND_MODE_MIRROR = 2

		,MP1_EXTEND_MODE_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_BITMAP_INTERPOLATION_MODE
	{
		MP1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR = 0

		,MP1_BITMAP_INTERPOLATION_MODE_LINEAR = 1

		,MP1_BITMAP_INTERPOLATION_MODE_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_DRAW_TEXT_OPTIONS
	{
		MP1_DRAW_TEXT_OPTIONS_NO_SNAP = 1

		,MP1_DRAW_TEXT_OPTIONS_CLIP = 2

		,MP1_DRAW_TEXT_OPTIONS_NONE = 0

		,MP1_DRAW_TEXT_OPTIONS_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_ARC_SIZE
	{
		MP1_ARC_SIZE_SMALL = 0

		,MP1_ARC_SIZE_LARGE = 1

		,MP1_ARC_SIZE_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_CAP_STYLE
	{
		MP1_CAP_STYLE_FLAT = 0

		,MP1_CAP_STYLE_SQUARE = 1

		,MP1_CAP_STYLE_ROUND = 2

		,MP1_CAP_STYLE_TRIANGLE = 3

		,MP1_CAP_STYLE_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_DASH_STYLE
	{
		MP1_DASH_STYLE_SOLID = 0

		,MP1_DASH_STYLE_DASH = 1

		,MP1_DASH_STYLE_DOT = 2

		,MP1_DASH_STYLE_DASH_DOT = 3

		,MP1_DASH_STYLE_DASH_DOT_DOT = 4

		,MP1_DASH_STYLE_CUSTOM = 5

		,MP1_DASH_STYLE_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_STROKE_TRANSFORM_TYPE
	{

		//
		// The stroke respects the world transform, the DPI, and the stroke width.
		//
		MP1_STROKE_TRANSFORM_TYPE_NORMAL = 0,

		//
		// The stroke does not respect the world transform, but it does respect the DPI and
		// the stroke width.
		//
		MP1_STROKE_TRANSFORM_TYPE_FIXED = 1,

		//
		// The stroke is forced to one pixel wide.
		//
		MP1_STROKE_TRANSFORM_TYPE_HAIRLINE = 2,
		MP1_STROKE_TRANSFORM_TYPE_FORCE_DWORD = 0xffffffff
	};

	enum MP1_LINE_JOIN
	{
		MP1_LINE_JOIN_MITER = 0

		,MP1_LINE_JOIN_BEVEL = 1

		,MP1_LINE_JOIN_ROUND = 2

		,MP1_LINE_JOIN_MITER_OR_BEVEL = 3

		,MP1_LINE_JOIN_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_COMBINE_MODE
	{
		MP1_COMBINE_MODE_UNION = 0

		,MP1_COMBINE_MODE_INTERSECT = 1

		,MP1_COMBINE_MODE_XOR = 2

		,MP1_COMBINE_MODE_EXCLUDE = 3

		,MP1_COMBINE_MODE_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_FIGURE_BEGIN
	{
		MP1_FIGURE_BEGIN_FILLED = 0

		,MP1_FIGURE_BEGIN_HOLLOW = 1

		,MP1_FIGURE_BEGIN_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_FIGURE_END
	{
		MP1_FIGURE_END_OPEN = 0

		,MP1_FIGURE_END_CLOSED = 1

		,MP1_FIGURE_END_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_PATH_SEGMENT
	{
		MP1_PATH_SEGMENT_NONE = 0

		,MP1_PATH_SEGMENT_FORCE_UNSTROKED = 1

		,MP1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN = 2

		,MP1_PATH_SEGMENT_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_SWEEP_DIRECTION
	{
		MP1_SWEEP_DIRECTION_COUNTER_CLOCKWISE = 0

		,MP1_SWEEP_DIRECTION_CLOCKWISE = 1

		,MP1_SWEEP_DIRECTION_FORCE_DWORD = 0x7fffffff
	};

	enum MP1_FILL_MODE
	{
		MP1_FILL_MODE_ALTERNATE = 0

		,MP1_FILL_MODE_WINDING = 1

		,MP1_FILL_MODE_FORCE_DWORD = 0x7fffffff
	};

	struct MP1_COLOR
	{
		float r;
		float g;
		float b;
		float a;
	};

// Export structs
/*
unsigned, can't handle negative points. not much practical use.
	struct MP1_POINT_2U
	{
		uint32_t x;
		uint32_t y;
	};
*/
	struct MP1_POINT
	{
		float x;
		float y;
	};

	struct MP1_POINT_L
	{
		int32_t x;
		int32_t y;
	};

	struct MP1_RECT
	{
		float left;
		float top;
		float right;
		float bottom;
	};

	/*
	struct MP1_RECT_U
	{
		uint32_t left;
		uint32_t top;
		uint32_t right;
		uint32_t bottom;
	};
	*/

	struct MP1_RECT_L
	{
		int32_t    left;
		int32_t    top;
		int32_t    right;
		int32_t    bottom;
	};

	struct MP1_SIZE
	{
		float width;
		float height;
	};

	struct MP1_SIZE_U
	{
		uint32_t width;
		uint32_t height;
	};

	struct MP1_SIZE_L
	{
		int32_t width;
		int32_t height;
	};

	struct MP1_MATRIX_3X2
	{
		float _11;
		float _12;
		float _21;
		float _22;
		float _31;
		float _32;
	};

	//struct MP1_PIXEL_FORMAT
	//{
	//	DXGI_FORMAT format;
	//	MP1_ALPHA_MODE alphaMode;
	//};

	struct MP1_BITMAP_PROPERTIES
	{
//		MP1_PIXEL_FORMAT pixelFormat;
		float dpiX;
		float dpiY;
	};

	struct MP1_GRADIENT_STOP
	{
		float position;
		MP1_COLOR color;
	};

	struct MP1_BRUSH_PROPERTIES
	{
		float opacity;
		MP1_MATRIX_3X2 transform;
	};

	struct MP1_BITMAP_BRUSH_PROPERTIES
	{
		MP1_EXTEND_MODE extendModeX;
		MP1_EXTEND_MODE extendModeY;
		MP1_BITMAP_INTERPOLATION_MODE interpolationMode;
	};

	struct MP1_LINEAR_GRADIENT_BRUSH_PROPERTIES
	{
		MP1_POINT startPoint;
		MP1_POINT endPoint;
	};

	struct MP1_RADIAL_GRADIENT_BRUSH_PROPERTIES
	{
		MP1_POINT center;
		MP1_POINT gradientOriginOffset;
		float radiusX;
		float radiusY;
	};

	struct MP1_BEZIER_SEGMENT
	{
		MP1_POINT point1;
		MP1_POINT point2;
		MP1_POINT point3;
	};

	struct MP1_TRIANGLE
	{
		MP1_POINT point1;
		MP1_POINT point2;
		MP1_POINT point3;
	};

	struct MP1_ARC_SEGMENT
	{
		MP1_POINT point;
		MP1_SIZE size;
		float rotationAngle;
		MP1_SWEEP_DIRECTION sweepDirection;
		MP1_ARC_SIZE arcSize;
	};

	struct MP1_QUADRATIC_BEZIER_SEGMENT
	{
		MP1_POINT point1;
		MP1_POINT point2;
	};

	struct MP1_ELLIPSE
	{
		MP1_POINT point;
		float radiusX;
		float radiusY;
	};

	struct MP1_ROUNDED_RECT
	{
		MP1_RECT rect;
		float radiusX;
		float radiusY;
	};

	struct MP1_STROKE_STYLE_PROPERTIES
	{
		MP1_CAP_STYLE startCap;
		MP1_CAP_STYLE endCap;
		MP1_CAP_STYLE dashCap;
		MP1_LINE_JOIN lineJoin;
		float miterLimit;
		MP1_DASH_STYLE dashStyle;
		float dashOffset;

		MP1_STROKE_TRANSFORM_TYPE transformType;
	};
// Interfaces
////////////////// FONTS //////////////////////////
// mimicks DWRITE_FONT_METRICS, except measurements are in DIPs not design units.
	struct MP1_FONT_METRICS
	{
		float ascent;
		float descent;
		float lineGap;
		float capHeight;
		float xHeight;
		float underlinePosition;
		float underlineThickness;
		float strikethroughPosition;
		float strikethroughThickness;
	};

	class DECLSPEC_NOVTABLE IMpTextFormat : public gmpi::IMpUnknown
	{
	public:
		virtual int32_t MP_STDCALL SetTextAlignment(MP1_TEXT_ALIGNMENT textAlignment) = 0;

		virtual int32_t MP_STDCALL SetParagraphAlignment(MP1_PARAGRAPH_ALIGNMENT paragraphAlignment) = 0;

		virtual int32_t MP_STDCALL SetWordWrapping(MP1_WORD_WRAPPING wordWrapping) = 0;

		virtual void MP_STDCALL GetTextExtentU(const char* utf8String, int32_t stringLength, MP1_SIZE* returnSize) = 0;

		virtual int32_t MP_STDCALL GetFontMetrics(MP1_FONT_METRICS* returnFontMetrics) = 0;

		// For the default method use lineSpacing=-1 (spacing depends solely on the content). For uniform spacing, the specified line height overrides the content.
		virtual int32_t MP_STDCALL SetLineSpacing(float lineSpacing, float baseline) = 0;
	};
	// GUID for ITextFormat
	// {ED903255-3FE0-4CE4-8CD1-97D72D51B7CB}
	static const gmpi::MpGuid SE_IID_TEXTFORMAT_MPGUI =
	{ 0xed903255, 0x3fe0, 0x4ce4,{ 0x8c, 0xd1, 0x97, 0xd7, 0x2d, 0x51, 0xb7, 0xcb } };

	class IMpFactory;

	class DECLSPEC_NOVTABLE IMpResource : public gmpi::IMpUnknown
	{
	public:
		virtual void MP_STDCALL GetFactory(IMpFactory** factory) = 0;
	};
	// GUID for IResource
	// {4540C6EE-98AB-4C79-B857-66AE64249125}
	//static const gmpi::MpGuid SE_IID_RESOURCE_MPGUI =
	//{ replaceme, 0x98ab, 0x4c79,{ 0xb8, 0x57, 0x66, 0xae, 0x64, 0x24, 0x91, 0x25 } };


	// GMPI bitmap pixel access.
	class DECLSPEC_NOVTABLE IMpBitmapPixels : public gmpi::IMpUnknown
	{
	public:
		enum PixelFormat {
			kARGB,
			kRGBA,
			kABGR,
			kBGRA,
			kBGRA_SRGB
		};
		virtual uint8_t* MP_STDCALL getAddress() const = 0;
		virtual int32_t MP_STDCALL getBytesPerRow() const = 0;
		virtual int32_t MP_STDCALL getPixelFormat() const = 0;
	};
	// GUID
	// {CCE4F628-289E-4EAB-9837-1755D9E5F793}
	//	static const gmpi::MpGuid SE_IID _GRAPHICS_BITMAP_PIXELS =
	static const gmpi::MpGuid SE_IID_BITMAP_PIXELS_MPGUI =
	{ 0xcce4f628, 0x289e, 0x4eab,{ 0x98, 0x37, 0x17, 0x55, 0xd9, 0xe5, 0xf7, 0x93 } };


	class DECLSPEC_NOVTABLE IMpBitmap : public IMpResource
	{
	public:
		// Float size. DEPRECATED: Should be integer size to avoid costly float->int conversions and for Direct2D compatibility.
		virtual MP1_SIZE MP_STDCALL GetSizeF() = 0;
		// Deprecated, see lockPixels.
		virtual int32_t MP_STDCALL lockPixelsOld(IMpBitmapPixels** returnPixels, bool alphaPremultiplied = false) = 0;

		// Deprecated. DirectX 11 has SRGB support.
		virtual void MP_STDCALL ApplyAlphaCorrection() = 0;

		// Integer size.
		virtual int32_t MP_STDCALL GetSize(MP1_SIZE_U* returnSize) = 0;

		// Same as lock pixels but with option to avoid overhead of copying pixels back into image. See MP1_BITMAP_LOCK_FLAGS.
		virtual int32_t MP_STDCALL lockPixels(IMpBitmapPixels** returnPixels, int32_t flags) = 0;
	};
	// GUID for IBitmap
	// {EDF250B7-29FE-4FEC-8C6A-FBCB1F0A301A}
	static const gmpi::MpGuid SE_IID_BITMAP_MPGUI =
	{ 0xedf250b7, 0x29fe, 0x4fec,{ 0x8c, 0x6a, 0xfb, 0xcb, 0x1f, 0xa, 0x30, 0x1a } };


	class DECLSPEC_NOVTABLE IMpGradientStopCollection : public IMpResource
	{
	public:
//		virtual uint32_t MP_STDCALL GetGradientStopCount() = 0;

	};
	// GUID for IGradientStopCollection
	// {AEE31225-BFF4-42DE-B8CA-233C5A3441CB}
	static const gmpi::MpGuid SE_IID_GRADIENTSTOPCOLLECTION_MPGUI =
	{ 0xaee31225, 0xbff4, 0x42de,{ 0xb8, 0xca, 0x23, 0x3c, 0x5a, 0x34, 0x41, 0xcb } };

	class DECLSPEC_NOVTABLE IMpBrush : public IMpResource
	{
	public:
	};
	// GUID for IBrush
	// {4540C6EE-98AB-4C79-B857-66AE64249125}
	//static const gmpi::MpGuid SE_IID_BRUSH_MPGUI =
	//{ replaceme, 0x98ab, 0x4c79,{ 0xb8, 0x57, 0x66, 0xae, 0x64, 0x24, 0x91, 0x25 } };

	class DECLSPEC_NOVTABLE IMpBitmapBrush : public IMpBrush
	{
	public:
		virtual void MP_STDCALL SetExtendModeX(MP1_EXTEND_MODE extendModeX) = 0;

		virtual void MP_STDCALL SetExtendModeY(MP1_EXTEND_MODE extendModeY) = 0;

		virtual void MP_STDCALL SetInterpolationMode(MP1_BITMAP_INTERPOLATION_MODE interpolationMode) = 0;
	};
	// GUID for IBitmapBrush
	// {4540C6EE-98AB-4C79-B857-66AE64249125}
	//static const gmpi::MpGuid SE_IID_BITMAPBRUSH_MPGUI =
	//{ replaceme, 0x98ab, 0x4c79,{ 0xb8, 0x57, 0x66, 0xae, 0x64, 0x24, 0x91, 0x25 } };

	class DECLSPEC_NOVTABLE IMpSolidColorBrush : public IMpBrush
	{
	public:
		virtual void MP_STDCALL SetColor(const MP1_COLOR* color) = 0;

		virtual MP1_COLOR MP_STDCALL GetColor() = 0;
	};
	// GUID for IMpSolidColorBrush
	// {BB3FD251-47A0-4273-90AB-A5CDC88F57B9}
	static const gmpi::MpGuid SE_IID_SOLIDCOLORBRUSH_MPGUI =
	{ 0xbb3fd251, 0x47a0, 0x4273,{ 0x90, 0xab, 0xa5, 0xcd, 0xc8, 0x8f, 0x57, 0xb9 } };

	class DECLSPEC_NOVTABLE IMpLinearGradientBrush : public IMpBrush
	{
	public:
		virtual void MP_STDCALL SetStartPoint(MP1_POINT startPoint) = 0;

		virtual void MP_STDCALL SetEndPoint(MP1_POINT endPoint) = 0;

//		virtual void MP_STDCALL GetGradientStopCollection(IMpGradientStopCollection** gradientStopCollection) = 0;
	};
	// GUID for ILinearGradientBrush
	// {986C3B9A-9D0A-4BF5-B721-0B9611B2798D}
	static const gmpi::MpGuid SE_IID_LINEARGRADIENTBRUSH_MPGUI =
	{ 0x986c3b9a, 0x9d0a, 0x4bf5,{ 0xb7, 0x21, 0xb, 0x96, 0x11, 0xb2, 0x79, 0x8d } };

	class DECLSPEC_NOVTABLE IMpRadialGradientBrush : public IMpBrush
	{
	public:
		virtual void MP_STDCALL SetCenter(MP1_POINT center) = 0;

		virtual void MP_STDCALL SetGradientOriginOffset(MP1_POINT gradientOriginOffset) = 0;

		virtual void MP_STDCALL SetRadiusX(float radiusX) = 0;

		virtual void MP_STDCALL SetRadiusY(float radiusY) = 0;
	};
	// GUID for IRadialGradientBrush
	// {A3436B5B-C3F7-4A27-9BD9-710D653EE560}
	static const gmpi::MpGuid SE_IID_RADIALGRADIENTBRUSH_MPGUI =
	{ 0xa3436b5b, 0xc3f7, 0x4a27,{ 0x9b, 0xd9, 0x71, 0xd, 0x65, 0x3e, 0xe5, 0x60 } };

	class DECLSPEC_NOVTABLE IMpStrokeStyle : public IMpResource
	{
	public:
		virtual MP1_CAP_STYLE MP_STDCALL GetStartCap() = 0;

		virtual MP1_CAP_STYLE MP_STDCALL GetEndCap() = 0;

		virtual MP1_CAP_STYLE MP_STDCALL GetDashCap() = 0;

		virtual float MP_STDCALL GetMiterLimit() = 0;

		virtual MP1_LINE_JOIN MP_STDCALL GetLineJoin() = 0;

		virtual float MP_STDCALL GetDashOffset() = 0;

		virtual MP1_DASH_STYLE MP_STDCALL GetDashStyle() = 0;

		virtual uint32_t MP_STDCALL GetDashesCount() = 0;

		virtual void MP_STDCALL GetDashes(float* dashes, uint32_t dashesCount) = 0;
	};
	// GUID for IStrokeStyle
	// {27D19BF3-9DB2-49CC-A8EE-28E0716EA8B6}
	static const gmpi::MpGuid SE_IID_STROKESTYLE_MPGUI =
	{ 0x27d19bf3, 0x9db2, 0x49cc,{ 0xa8, 0xee, 0x28, 0xe0, 0x71, 0x6e, 0xa8, 0xb6 } };

	/*
	class DECLSPEC_NOVTABLE IMpGeometry : public IMpResource
	{
	public:
	};
	// GUID for IGeometry
	// {4540C6EE-98AB-4C79-B857-66AE64249125}
	//static const gmpi::MpGuid SE_IID_GEOMETRY_MPGUI =
	//{ replaceme, 0x98ab, 0x4c79,{ 0xb8, 0x57, 0x66, 0xae, 0x64, 0x24, 0x91, 0x25 } };

	
	class DECLSPEC_NOVTABLE IMpRectangleGeometry : public IMpGeometry
	{
	public:
		virtual void MP_STDCALL GetRect(MP1_RECT* rect) = 0;
	};
	// GUID for IRectangleGeometry
	// {4540C6EE-98AB-4C79-B857-66AE64249125}
	//static const gmpi::MpGuid SE_IID_RECTANGLEGEOMETRY_MPGUI =
	//{ replaceme, 0x98ab, 0x4c79,{ 0xb8, 0x57, 0x66, 0xae, 0x64, 0x24, 0x91, 0x25 } };

	class DECLSPEC_NOVTABLE IMpRoundedRectangleGeometry : public IMpGeometry
	{
	public:
		virtual void MP_STDCALL GetRoundedRect(MP1_ROUNDED_RECT* roundedRect) = 0;
	};
	// GUID for IRoundedRectangleGeometry
	// {4540C6EE-98AB-4C79-B857-66AE64249125}
	//static const gmpi::MpGuid SE_IID_ROUNDEDRECTANGLEGEOMETRY_MPGUI =
	//{ replaceme, 0x98ab, 0x4c79,{ 0xb8, 0x57, 0x66, 0xae, 0x64, 0x24, 0x91, 0x25 } };

	class DECLSPEC_NOVTABLE IMpEllipseGeometry : public IMpGeometry
	{
	public:
		virtual void MP_STDCALL GetEllipse(MP1_ELLIPSE* ellipse) = 0;
	};
	// GUID for IEllipseGeometry
	// {4540C6EE-98AB-4C79-B857-66AE64249125}
	//static const gmpi::MpGuid SE_IID_ELLIPSEGEOMETRY_MPGUI =
	//{ replaceme, 0x98ab, 0x4c79,{ 0xb8, 0x57, 0x66, 0xae, 0x64, 0x24, 0x91, 0x25 } };
	*/

	class DECLSPEC_NOVTABLE IMpSimplifiedGeometrySink : public gmpi::IMpUnknown
	{
	public:
		virtual void MP_STDCALL BeginFigure(MP1_POINT startPoint, MP1_FIGURE_BEGIN figureBegin) = 0;

		virtual void MP_STDCALL AddLines(const MP1_POINT* points, uint32_t pointsCount) = 0;

		virtual void MP_STDCALL AddBeziers(const MP1_BEZIER_SEGMENT* beziers, uint32_t beziersCount) = 0;

		virtual void MP_STDCALL EndFigure(MP1_FIGURE_END figureEnd) = 0;

		virtual int32_t MP_STDCALL Close() = 0;
	};
	// GUID for ISimplifiedGeometrySink
	// {4540C6EE-98AB-4C79-B857-66AE64249125}
	//static const gmpi::MpGuid SE_IID_SIMPLIFIEDGEOMETRYSINK_MPGUI =
	//{ replaceme, 0x98ab, 0x4c79,{ 0xb8, 0x57, 0x66, 0xae, 0x64, 0x24, 0x91, 0x25 } };

	class DECLSPEC_NOVTABLE IMpGeometrySink : public IMpSimplifiedGeometrySink
	{
	public:
		virtual void MP_STDCALL AddLine(MP1_POINT point) = 0;

		virtual void MP_STDCALL AddBezier(const MP1_BEZIER_SEGMENT* bezier) = 0;

		virtual void MP_STDCALL AddQuadraticBezier(const MP1_QUADRATIC_BEZIER_SEGMENT* bezier) = 0;

		virtual void MP_STDCALL AddQuadraticBeziers(const MP1_QUADRATIC_BEZIER_SEGMENT* beziers, uint32_t beziersCount) = 0;

		virtual void MP_STDCALL AddArc(const MP1_ARC_SEGMENT* arc) = 0;
	};
	// GUID for IGeometrySink
	// {10385F43-03C3-436B-B6B0-74A4EC617A22}
	static const gmpi::MpGuid SE_IID_GEOMETRYSINK_MPGUI =
	{ 0x10385f43, 0x3c3, 0x436b,{ 0xb6, 0xb0, 0x74, 0xa4, 0xec, 0x61, 0x7a, 0x22 } };

	class DECLSPEC_NOVTABLE IMpPathGeometry : public IMpResource //IMpGeometry
	{
	public:
		virtual int32_t MP_STDCALL Open(IMpGeometrySink** geometrySink) = 0;

		// in DX, these are part of IMpGeometry. But were added later here, and so added last. not a big deal since we support only one type of geometry, not many like DX.
		virtual int32_t MP_STDCALL StrokeContainsPoint(MP1_POINT point, float strokeWidth, IMpStrokeStyle* strokeStyle, const MP1_MATRIX_3X2* worldTransform, bool* returnContains) = 0;
		virtual int32_t MP_STDCALL FillContainsPoint(MP1_POINT point, const MP1_MATRIX_3X2* worldTransform, bool* returnContains) = 0;
		virtual int32_t MP_STDCALL GetWidenedBounds(float strokeWidth, IMpStrokeStyle* strokeStyle, const MP1_MATRIX_3X2* worldTransform, MP1_RECT* returnBounds) = 0;
	};
	// GUID for IMpPathGeometry
	// {89C6E868-B8A5-49BF-B771-02FB1EEF38AD}
	static const gmpi::MpGuid SE_IID_PATHGEOMETRY_MPGUI =
	{ 0x89c6e868, 0xb8a5, 0x49bf,{ 0xb7, 0x71, 0x2, 0xfb, 0x1e, 0xef, 0x38, 0xad } };


	class DECLSPEC_NOVTABLE IMpTessellationSink : public gmpi::IMpUnknown
	{
	public:
		virtual void MP_STDCALL AddTriangles(const MP1_TRIANGLE* triangles, uint32_t trianglesCount) = 0;

		virtual int32_t MP_STDCALL Close() = 0;
	};
	// GUID for ITessellationSink
	// {40110DEB-90AE-4168-AF53-E79363B71692}
	static const gmpi::MpGuid SE_IID_TESSELLATIONSINK_MPGUI =
	{ 0x40110deb, 0x90ae, 0x4168,{ 0xaf, 0x53, 0xe7, 0x93, 0x63, 0xb7, 0x16, 0x92 } };

	class DECLSPEC_NOVTABLE IMpMesh : public IMpResource
	{
	public:
		virtual int32_t MP_STDCALL Open(IMpTessellationSink** tessellationSink) = 0;
	};
	// GUID for IMesh
	// {92063AC3-8CE6-4A7D-9858-E22F03782B16}
	static const gmpi::MpGuid SE_IID_MESH_MPGUI =
	{ 0x92063ac3, 0x8ce6, 0x4a7d, { 0x98, 0x58, 0xe2, 0x2f, 0x3, 0x78, 0x2b, 0x16 } };

	//class DECLSPEC_NOVTABLE IMpDrawingStateBlock : public IMpResource
	//{
	//public:
	//	virtual void MP_STDCALL GetDescription(MP1_DRAWING_STATE_DESCRIPTION* stateDescription) = 0;

	//	virtual void MP_STDCALL SetDescription(MP1_DRAWING_STATE_DESCRIPTION* stateDescription) = 0;

	//	virtual void MP_STDCALL SetTextRenderingParams(IMpRenderingParams* textRenderingParams) = 0;

	//	virtual void MP_STDCALL GetTextRenderingParams(IMpRenderingParams** textRenderingParams) = 0;
	//};
	// GUID for IDrawingStateBlock
	// {4540C6EE-98AB-4C79-B857-66AE64249125}
	//static const gmpi::MpGuid SE_IID_DRAWINGSTATEBLOCK_MPGUI =
	//{ replaceme, 0x98ab, 0x4c79,{ 0xb8, 0x57, 0x66, 0xae, 0x64, 0x24, 0x91, 0x25 } };

/* FAIL
	class DECLSPEC_NOVTABLE IUpdateRegion : public gmpi::IMpUnknown
	{
	public:
		virtual bool MP_STDCALL isVisible(GmpiDrawing_API::MP1_RECT* rect) = 0;
		virtual int32_t MP_STDCALL getUpdateRects(GmpiDrawing_API::MP1_RECT*** rect) = 0;
	};
	// GUID for IUpdateRegion
	// {ED046C45-6DD5-488C-BE6F-3D7B47DFFE27}
	static const gmpi::MpGuid SE_IID_UPDATE_REGION_MPGUI =
	{ 0xed046c45, 0x6dd5, 0x488c,{ 0xbe, 0x6f, 0x3d, 0x7b, 0x47, 0xdf, 0xfe, 0x27 } };
*/


	class DECLSPEC_NOVTABLE IMpDeviceContext : public IMpResource
	{
	public:
//		virtual int32_t MP_STDCALL CreateBitmap(MP1_SIZE_U size, const MP1_BITMAP_PROPERTIES* bitmapProperties, IMpBitmap** bitmap) = 0;

		virtual int32_t MP_STDCALL CreateBitmapBrush(const IMpBitmap* bitmap, const MP1_BITMAP_BRUSH_PROPERTIES* bitmapBrushProperties, const MP1_BRUSH_PROPERTIES* brushProperties, IMpBitmapBrush** bitmapBrush) = 0;

		virtual int32_t MP_STDCALL CreateSolidColorBrush(const MP1_COLOR* color, /*MP1_BRUSH_PROPERTIES* brushProperties,*/ IMpSolidColorBrush** solidColorBrush) = 0;

		virtual int32_t MP_STDCALL CreateGradientStopCollection(const MP1_GRADIENT_STOP* gradientStops, uint32_t gradientStopsCount,/* ? MP1_GAMMA colorInterpolationGamma, MP1_EXTEND_MODE extendMode,*/ IMpGradientStopCollection** gradientStopCollection) = 0;

		virtual int32_t MP_STDCALL CreateLinearGradientBrush(const MP1_LINEAR_GRADIENT_BRUSH_PROPERTIES* linearGradientBrushProperties, const MP1_BRUSH_PROPERTIES* brushProperties, const IMpGradientStopCollection* gradientStopCollection, IMpLinearGradientBrush** linearGradientBrush) = 0;

		virtual int32_t MP_STDCALL CreateRadialGradientBrush(const MP1_RADIAL_GRADIENT_BRUSH_PROPERTIES* radialGradientBrushProperties, const MP1_BRUSH_PROPERTIES* brushProperties, const IMpGradientStopCollection* gradientStopCollection, IMpRadialGradientBrush** radialGradientBrush) = 0;

		virtual void MP_STDCALL DrawLine(MP1_POINT point0, MP1_POINT point1, const IMpBrush* brush, float strokeWidth = 1.0f, const IMpStrokeStyle* strokeStyle = nullptr) = 0;

		virtual void MP_STDCALL DrawRectangle(const MP1_RECT* rect, const IMpBrush* brush, float strokeWidth = 1.0f, const IMpStrokeStyle* strokeStyle = nullptr) = 0;

		virtual void MP_STDCALL FillRectangle(const MP1_RECT* rect, const IMpBrush* brush) = 0;

		virtual void MP_STDCALL DrawRoundedRectangle(const MP1_ROUNDED_RECT* roundedRect, const IMpBrush* brush, float strokeWidth = 1.0f, const IMpStrokeStyle* strokeStyle = nullptr) = 0;

		virtual void MP_STDCALL FillRoundedRectangle(const MP1_ROUNDED_RECT* roundedRect, const IMpBrush* brush) = 0;

		virtual void MP_STDCALL DrawEllipse(const MP1_ELLIPSE* ellipse, const IMpBrush* brush, float strokeWidth = 1.0f, const IMpStrokeStyle* strokeStyle = nullptr) = 0;

		virtual void MP_STDCALL FillEllipse(const MP1_ELLIPSE* ellipse, const IMpBrush* brush) = 0;

		virtual void MP_STDCALL DrawGeometry(const IMpPathGeometry* geometry, const IMpBrush* brush, float strokeWidth = 1.0f, const IMpStrokeStyle* strokeStyle = nullptr) = 0;

		virtual void MP_STDCALL FillGeometry(const IMpPathGeometry* geometry, const IMpBrush* brush, const IMpBrush* opacityBrush = nullptr) = 0;

		virtual void MP_STDCALL DrawBitmap(const IMpBitmap* bitmap, const MP1_RECT* destinationRectangle, float opacity, /* MP1_BITMAP_INTERPOLATION_MODE*/ int32_t interpolationMode, const MP1_RECT* sourceRectangle) = 0;

		virtual void MP_STDCALL DrawTextU(const char* utf8String, int32_t stringLength, const IMpTextFormat* textFormat, const MP1_RECT* layoutRect, const IMpBrush* brush, int32_t flags = 0) = 0;

		virtual void MP_STDCALL SetTransform(const MP1_MATRIX_3X2* transform) = 0;

		virtual void MP_STDCALL GetTransform(MP1_MATRIX_3X2* transform) = 0;

		virtual void MP_STDCALL PushAxisAlignedClip(const MP1_RECT* clipRect/*, MP1_ANTIALIAS_MODE antialiasMode*/) = 0;

		virtual void MP_STDCALL PopAxisAlignedClip() = 0;

		virtual void MP_STDCALL GetAxisAlignedClip(MP1_RECT* returnClipRect) = 0;

		virtual void MP_STDCALL Clear(const MP1_COLOR* clearColor) = 0;

		virtual void MP_STDCALL BeginDraw() = 0;

		virtual int32_t MP_STDCALL EndDraw() = 0;

		virtual int32_t MP_STDCALL CreateCompatibleRenderTarget(const MP1_SIZE* desiredSize, class IMpBitmapRenderTarget** bitmapRenderTarget) = 0;

//		virtual int32_t MP_STDCALL CreateMesh(GmpiDrawing_API::IMpMesh** returnObject) = 0;

//		virtual void MP_STDCALL FillMesh(const GmpiDrawing_API::IMpMesh* mesh, const GmpiDrawing_API::IMpBrush* brush) = 0;

//		virtual int32_t GetUpdateRegion(IUpdateRegion** returnUpdateRegion) = 0;
	};

	// GUID for IGraphics
	// {A1D9751D-0C43-4F57-8958-E0BCE359B2FD}
	static const gmpi::MpGuid SE_IID_DEVICECONTEXT_MPGUI =
	{ 0xa1d9751d, 0xc43, 0x4f57,{ 0x89, 0x58, 0xe0, 0xbc, 0xe3, 0x59, 0xb2, 0xfd } };


	class DECLSPEC_NOVTABLE IMpBitmapRenderTarget : public IMpDeviceContext
	{
	public:
		virtual int32_t MP_STDCALL GetBitmap(IMpBitmap** returnBitmap) = 0;
	};

	// GUID for IMpBitmapRenderTarget
	// {242DC082-399A-4CAF-8782-878134502F99}
	static const gmpi::MpGuid SE_IID_BITMAP_RENDERTARGET_MPGUI =
	{ 0x242dc082, 0x399a, 0x4caf,{ 0x87, 0x82, 0x87, 0x81, 0x34, 0x50, 0x2f, 0x99 } };


	class DECLSPEC_NOVTABLE IMpFactory : public gmpi::IMpUnknown
	{
	public:
		virtual int32_t MP_STDCALL CreatePathGeometry(GmpiDrawing_API::IMpPathGeometry** pathGeometry) = 0;
		virtual int32_t MP_STDCALL CreateTextFormat(const char* fontFamilyName, void* unused /* fontCollection */, GmpiDrawing_API::MP1_FONT_WEIGHT fontWeight, GmpiDrawing_API::MP1_FONT_STYLE fontStyle, GmpiDrawing_API::MP1_FONT_STRETCH fontStretch, float fontSize, void* unused2 /* localeName */, GmpiDrawing_API::IMpTextFormat** textFormat) = 0;
		virtual int32_t MP_STDCALL CreateImage(int32_t width, int32_t height, GmpiDrawing_API::IMpBitmap** returnDiBitmap) = 0;

		// test for winrt. perhaps uri could indicate if image is in resources, and could use stream internally if nesc (i.e. VST2 only.) or just write it to disk temp.
		// LoadStreamImage would be private member, not on store apps.
		virtual int32_t MP_STDCALL LoadImageU(const char* utf8Uri, GmpiDrawing_API::IMpBitmap** returnDiBitmap) = 0;
//		virtual int32_t MP_STDCALL LoadStreamImage(gmpi::IMpUnknown* stream, void** returnDiBitmap) = 0; // DEPRECATED. TO BE REMOVED IN FINAL, USE LoadImage().
		virtual int32_t MP_STDCALL CreateStrokeStyle(const GmpiDrawing_API::MP1_STROKE_STYLE_PROPERTIES* strokeStyleProperties, float* dashes, int32_t dashesCount, GmpiDrawing_API::IMpStrokeStyle** returnValue) = 0;
	};

	// GUID for IMpFactory
	// {481D4609-E28B-4698-BB2D-6480475B8F31}
	static const gmpi::MpGuid SE_IID_FACTORY_MPGUI =
	{ 0x481d4609, 0xe28b, 0x4698,{ 0xbb, 0x2d, 0x64, 0x80, 0x47, 0x5b, 0x8f, 0x31 } };

	// GUID for GmpiDrawing_API::IMpFactory
	// {F4F6ED95-CC5D-4DCE-AAA4-54A10AC9DA59}
	//static const gmpi::MpGuid SE_IID_GRAPHICS_FACTORY =
	//{ 0xf4f6ed95, 0xcc5d, 0x4dce,{ 0xaa, 0xa4, 0x54, 0xa1, 0xa, 0xc9, 0xda, 0x59 } };
} // namespace.

#endif //include
