#include "../sharedLegacyWidgets/SubControlBase.h"
#include "../se_sdk3/Drawing.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "../shared/xplatform_modifier_keys.h"

using namespace gmpi;
using namespace gmpi_gui;
using namespace GmpiDrawing;

class VectorBase : public SubControlBase
{
protected:
	FloatGuiPin pinAnimationPosition;
	StringGuiPin pinForeground;
	StringGuiPin pinBackground;
	BoolGuiPin pinReadOnly;

	GmpiDrawing_API::MP1_POINT pointPrevious;

	void onSetAnimationPosition()
	{
		invalidateRect();
	}

	VectorBase()
	{
		initializePin(pinAnimationPosition, static_cast<MpGuiBaseMemberPtr2>(&VectorBase::onSetAnimationPosition));
		initializePin(pinHint);
		initializePin(pinMenuItems);
		initializePin(pinMenuSelection);
		initializePin(pinMouseDown);
		initializePin(pinBackground, static_cast<MpGuiBaseMemberPtr2>(&VectorBase::onSetAnimationPosition));
		initializePin(pinForeground, static_cast<MpGuiBaseMemberPtr2>(&VectorBase::onSetAnimationPosition));
		initializePin(pinReadOnly);
	}

	int32_t MP_STDCALL onPointerDown(int32_t flags, GmpiDrawing_API::MP1_POINT point) override
	{
		// Let host handle right-clicks.
		if (pinReadOnly == true || (flags & GG_POINTER_FLAG_FIRSTBUTTON) == 0)
		{
			return gmpi::MP_OK; // Indicate successful hit, so right-click menu can show.
		}

		pointPrevious = point;	// note first point.

								//	smallDragSuppression = true;
		pinMouseDown = true;

		setCapture();

		return gmpi::MP_OK;
	}

	int32_t MP_STDCALL onPointerMove(int32_t flags, GmpiDrawing_API::MP1_POINT point) override
	{
		if (!getCapture())
		{
			return gmpi::MP_UNHANDLED;
		}

		Point offset(point.x - pointPrevious.x, point.y - pointPrevious.y); // TODO overload subtraction.

		float coarseness = 0.005f;

//		if (modifier_keys::isHeldCtrl()) // <cntr> key magnifies
		if (flags & gmpi_gui_api::GG_POINTER_KEY_CONTROL) // <cntr> key magnifies
			coarseness = 0.001f;

		float new_pos = pinAnimationPosition;
		new_pos = new_pos - coarseness * (float)offset.y;

		if (new_pos < 0.f)
			new_pos = 0.f;

		if (new_pos > 1.f)
			new_pos = 1.f;

		pinAnimationPosition = new_pos;

		pointPrevious = point;

		invalidateRect();

		return gmpi::MP_OK;
	}
};

class VectorRing : public VectorBase
{
public:

	void calcDimensions(Point& center, float& radius, float& thickness)
	{
		auto r = getRect();

		center = Point((r.left + r.right) * 0.5f, (r.top + r.bottom) * 0.5f);
		radius = (std::min)(r.getWidth(), r.getHeight()) * 0.4f;
		thickness = radius * 0.2f;
	}

	int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext ) override
	{
		Graphics g(drawingContext);

		Point center;
		float radius;
		float thickness;
		calcDimensions(center, radius, thickness);

		auto brush = g.CreateSolidColorBrush(Color::FromHexString(pinForeground));

		// inner gray circle
		auto dimBrush = g.CreateSolidColorBrush(Color::FromHexString(pinBackground));
		//		g.DrawCircle(center, radius - thickness, dimBrush, 2.0f);

		const float startAngle = 35.0f; // angle between "straight-down" and start of arc. In degrees.
		const float startAngleRadians = startAngle * M_PI / 180.f; // angle between "straight-down" and start of arc. In degrees.
		const float quarterTurnClockwise = M_PI * 0.5f;

		Point startPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians));
		StrokeStyleProperties strokeStyleProperties;
		strokeStyleProperties.setCapStyle(CapStyle::Round);
		auto strokeStyle = g.GetFactory().CreateStrokeStyle(strokeStyleProperties);

		// Background gray arc.
		{
			float sweepAngle = (M_PI * 2.0f - startAngleRadians * 2.0f);

			Point endPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians + sweepAngle), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians + sweepAngle));

			auto arcGeometry = g.GetFactory().CreatePathGeometry();
			auto sink = arcGeometry.Open();
			sink.BeginFigure(startPoint);
			sink.AddArc(ArcSegment(endPoint, Size(radius, radius), 0.0f, SweepDirection::Clockwise, sweepAngle > M_PI ? ArcSize::Large : ArcSize::Small));
			sink.EndFigure(FigureEnd::Open);
			sink.Close();

			g.DrawGeometry(arcGeometry, dimBrush, thickness, strokeStyle);
		}

		// foreground colored arc
		{
			float nomalised = pinAnimationPosition;
			float sweepAngle = nomalised * (static_cast<float>(M_PI) * 2.0f - startAngleRadians * 2.0f);

			Point endPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians + sweepAngle), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians + sweepAngle));

			auto arcGeometry = g.GetFactory().CreatePathGeometry();
			auto sink = arcGeometry.Open();
			sink.BeginFigure(startPoint);
			sink.AddArc(ArcSegment(endPoint, Size(radius, radius), 0.0f, SweepDirection::Clockwise, sweepAngle > M_PI ? ArcSize::Large : ArcSize::Small));
			sink.EndFigure(FigureEnd::Open);
			sink.Close();

			g.DrawGeometry(arcGeometry, brush, thickness, strokeStyle);
		}

		return gmpi::MP_OK;
	}

	int32_t MP_STDCALL hitTest(MP1_POINT point) override
	{
		Point center;
		float radius;
		float thickness;
		calcDimensions(center, radius, thickness);

		Point v(point.x - center.x, point.y - center.y);
		float distSquared = v.x * v.x + v.y * v.y;
		float outerSquared = (radius + thickness * 0.5f);
		outerSquared *= outerSquared;

		if (distSquared > outerSquared)
			return MP_FAIL;

		float innerSqared = (radius - thickness * 0.5f);
		innerSqared *= innerSqared;

		if (distSquared < innerSqared)
			return MP_FAIL;

		return MP_OK;
	}
};

class VectorKnob : public VectorRing
{
	int32_t MP_STDCALL hitTest(MP1_POINT point) override
	{
		Point center;
		float radius;
		float thickness;
		calcDimensions(center, radius, thickness);

		Point v(point.x - center.x, point.y - center.y);
		float distSquared = v.x * v.x + v.y * v.y;
		float outerSquared = (radius + thickness * 0.5f);
		outerSquared *= outerSquared;

		return distSquared <= outerSquared ? MP_OK : MP_FAIL;
	}

	int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext) override
	{
		Graphics g(drawingContext);

		Point center;
		float radius;
		float thickness;
		calcDimensions(center, radius, thickness);

		auto brush = g.CreateSolidColorBrush(Color::FromHexString(pinForeground));

		auto dimBrush = g.CreateSolidColorBrush(Color::FromHexString(pinBackground));

		const float startAngle = 35.0f; // angle between "straight-down" and start of arc. In degrees.
		const float startAngleRadians = startAngle * M_PI / 180.f; // angle between "straight-down" and start of arc. In degrees.
		const float quarterTurnClockwise = M_PI * 0.5f;

		StrokeStyleProperties strokeStyleProperties;
		strokeStyleProperties.setCapStyle(CapStyle::Round);
		strokeStyleProperties.setLineJoin(LineJoin::Round);
		auto strokeStyle = g.GetFactory().CreateStrokeStyle(strokeStyleProperties);

		Point startPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians));
		float sweepAngle = (M_PI * 2.0f - startAngleRadians * 2.0f);
		Point endPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians + sweepAngle), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians + sweepAngle));

		float nomalised = pinAnimationPosition;
		sweepAngle = nomalised * (static_cast<float>(M_PI) * 2.0f - startAngleRadians * 2.0f);
		Point movingPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians + sweepAngle), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians + sweepAngle));

		Size circleSize(radius, radius);

		// colored arc
		{
			auto arcGeometry = g.GetFactory().CreatePathGeometry();
			auto sink = arcGeometry.Open();
			sink.BeginFigure(startPoint);
			sink.AddArc(ArcSegment(movingPoint, circleSize, 0.0f, SweepDirection::Clockwise, sweepAngle > M_PI ? ArcSize::Large : ArcSize::Small));
			sink.EndFigure(FigureEnd::Open);
			sink.Close();

			g.DrawGeometry(arcGeometry, brush, thickness, strokeStyle);
		}

		// gray arc.
		{

			auto arcGeometry = g.GetFactory().CreatePathGeometry();
			auto sink = arcGeometry.Open();
			sink.BeginFigure(center);
			sink.AddLine(movingPoint);
			sink.AddArc(ArcSegment(endPoint, circleSize, 0.0f, SweepDirection::Clockwise, sweepAngle + startAngleRadians * 2.0f < M_PI ? ArcSize::Large : ArcSize::Small));

			sink.EndFigure(FigureEnd::Open);
			sink.Close();

			g.DrawGeometry(arcGeometry, dimBrush, thickness, strokeStyle);
		}

		return gmpi::MP_OK;
	}
};

class VectorPanKnob : public VectorKnob
{
	int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext) override
	{
		Graphics g(drawingContext);

		Point center;
		float radius;
		float thickness;
		calcDimensions(center, radius, thickness);

		auto brush = g.CreateSolidColorBrush(Color::FromHexString(pinForeground));

		auto dimBrush = g.CreateSolidColorBrush(Color::FromHexString(pinBackground));

		const float startAngle = 35.0f; // angle between "straight-down" and start of arc. In degrees.
		const float startAngleRadians = startAngle * M_PI / 180.f; // angle between "straight-down" and start of arc. In degrees.
		const float quarterTurnClockwise = M_PI * 0.5f;

		StrokeStyleProperties strokeStyleProperties;
		strokeStyleProperties.setCapStyle(CapStyle::Round);
		strokeStyleProperties.setLineJoin(LineJoin::Round);
		auto strokeStyle = g.GetFactory().CreateStrokeStyle(strokeStyleProperties);

		Point startPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians));
		Point midPoint(center.x, center.y - radius);
		float sweepAngle = (M_PI * 2.0f - startAngleRadians * 2.0f);
		Point endPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians + sweepAngle), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians + sweepAngle));

		float nomalised = pinAnimationPosition;
		sweepAngle = nomalised * (static_cast<float>(M_PI) * 2.0f - startAngleRadians * 2.0f);
		Point movingPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians + sweepAngle), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians + sweepAngle));

		Size circleSize(radius, radius);

		// Background gray half-circle.
		{
			auto arcGeometry = g.GetFactory().CreatePathGeometry();
			auto sink = arcGeometry.Open();
			sink.BeginFigure(midPoint);
			if (nomalised > 0.5f)
			{
				sink.AddArc(ArcSegment(startPoint, circleSize, 0.0f, SweepDirection::CounterClockwise, ArcSize::Small));
			}
			else
			{
				sink.AddArc(ArcSegment(endPoint, circleSize, 0.0f, SweepDirection::Clockwise, ArcSize::Small));
			}

			sink.EndFigure(FigureEnd::Open);
			sink.Close();

			g.DrawGeometry(arcGeometry, dimBrush, thickness, strokeStyle);
		}

		// colored arc
		{
			auto arcGeometry = g.GetFactory().CreatePathGeometry();
			auto sink = arcGeometry.Open();
			sink.BeginFigure(midPoint);
			if (nomalised < 0.5f)
			{
				sink.AddArc(ArcSegment(movingPoint, circleSize, 0.0f, SweepDirection::CounterClockwise, ArcSize::Small));
			}
			else
			{
				sink.AddArc(ArcSegment(movingPoint, circleSize, 0.0f, SweepDirection::Clockwise, ArcSize::Small));
			}
			sink.EndFigure(FigureEnd::Open);
			sink.Close();

			g.DrawGeometry(arcGeometry, brush, thickness);
		}

		// Background gray arc, plus center line.
		{
			auto arcGeometry = g.GetFactory().CreatePathGeometry();
			auto sink = arcGeometry.Open();
			sink.BeginFigure(center);
			sink.AddLine(movingPoint);
			if (nomalised < 0.5f)
			{
				sink.AddArc(ArcSegment(startPoint, circleSize, 0.0f, SweepDirection::CounterClockwise, ArcSize::Small));
			}
			else
			{
				sink.AddArc(ArcSegment(endPoint, circleSize, 0.0f, SweepDirection::Clockwise, ArcSize::Small));
			}

			sink.EndFigure(FigureEnd::Open);
			sink.Close();

			g.DrawGeometry(arcGeometry, dimBrush, thickness, strokeStyle);
		}

		return gmpi::MP_OK;
	}
};

class VectorKnob_VCV : public VectorKnob
{
	int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext) override
	{
		Graphics g(drawingContext);

		Point center;
		float radius;
		float thickness;
		calcDimensions(center, radius, thickness);

		auto brushForeground = g.CreateSolidColorBrush(Color::FromHexString(pinForeground));
		auto brushBackground = g.CreateSolidColorBrush(Color::FromHexString(pinBackground));

		const float startAngle = 35.0f; // angle between "straight-down" and start of arc. In degrees.
		const float startAngleRadians = startAngle * M_PI / 180.f; // angle between "straight-down" and start of arc. In degrees.
		const float quarterTurnClockwise = M_PI * 0.5f;

		StrokeStyleProperties strokeStyleProperties;
		strokeStyleProperties.setCapStyle(CapStyle::Round);
		strokeStyleProperties.setLineJoin(LineJoin::Round);
		auto strokeStyle = g.GetFactory().CreateStrokeStyle(strokeStyleProperties);

		Point startPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians));
		Point midPoint(center.x, center.y - radius);
		float sweepAngle = (M_PI * 2.0f - startAngleRadians * 2.0f);
		Point endPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians + sweepAngle), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians + sweepAngle));

		float nomalised = pinAnimationPosition;
		sweepAngle = nomalised * (static_cast<float>(M_PI) * 2.0f - startAngleRadians * 2.0f);
		Point movingPoint(center.x + radius * cosf(quarterTurnClockwise + startAngleRadians + sweepAngle), center.y + radius * sinf(quarterTurnClockwise + startAngleRadians + sweepAngle));

		Size circleSize(radius, radius);

		// Background circle.
		{
			g.FillCircle(center, radius + thickness * 0.5f, brushBackground);
		}

		// Line.
		{
			g.DrawLine(center, movingPoint, brushForeground, thickness, strokeStyle);
		}

		return gmpi::MP_OK;
	}
};

class VectorBar : public VectorBase
{
public:
	void calcDimensions(Point& startPoint, Point& endPoint, float& thickness)
	{
		auto r = getRect();

		thickness = 0.2f * (std::min)(r.getWidth(), r.getHeight()) * 0.4f;

		if (r.getWidth() > r.getHeight())
		{
			float mid = floorf(0.5f + (r.top + r.bottom) * 0.5f);
			startPoint = Point(r.left + thickness, mid);
			endPoint = Point(r.right - thickness, mid);
		}
		else
		{
			float mid = floorf(0.5f + (r.left + r.right) * 0.5f);
			startPoint = Point(mid, r.bottom - thickness);
			endPoint = Point(mid, r.top + thickness);
		}
	}

	int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext) override
	{
		Graphics g(drawingContext);

		auto r = getRect();

		Point startPoint;
		Point endPoint;
		float thickness;
		calcDimensions(startPoint, endPoint, thickness);

		auto brush = g.CreateSolidColorBrush(Color::FromHexString(pinForeground));
		auto dimBrush = g.CreateSolidColorBrush(Color::FromHexString(pinBackground));

		StrokeStyleProperties strokeStyleProperties;
		strokeStyleProperties.setCapStyle(CapStyle::Round);
		auto strokeStyle = g.GetFactory().CreateStrokeStyle(strokeStyleProperties);

		// Background gray bar.
		{
			g.DrawLine(startPoint, endPoint, dimBrush, thickness, strokeStyle);
		}

		// foreground colored bar
		float nomalised = pinAnimationPosition;
		if (nomalised > 0.001f)
		{
			Point meterPoint(startPoint.x + nomalised * (endPoint.x - startPoint.x), startPoint.y + nomalised * (endPoint.y - startPoint.y));
			g.DrawLine(startPoint, meterPoint, brush, thickness, strokeStyle);
		}

		return gmpi::MP_OK;
	}

	int32_t MP_STDCALL hitTest(MP1_POINT point) override
	{
		Point startPoint;
		Point endPoint;
		float thickness;
		calcDimensions(startPoint, endPoint, thickness);

		if (startPoint.x == endPoint.x)
		{
			return fabsf(point.x - startPoint.x) <= thickness * 0.5f && point.y >= endPoint.y - thickness * 0.5f && point.y <= startPoint.y + thickness * 0.5f ? gmpi::MP_OK : gmpi::MP_FAIL;
		}
		else
		{
			return fabsf(point.y - startPoint.y) <= thickness * 0.5f && point.x <= endPoint.x - thickness * 0.5f && point.x >= startPoint.x + thickness * 0.5f ? gmpi::MP_OK : gmpi::MP_FAIL;
		}
	}
};

namespace
{
	bool r[] =
	{
	Register<VectorBar>::withId(L"SE Vector Bar"),
	Register<VectorRing>::withId(L"SE Vector Ring"),
	Register<VectorKnob>::withId(L"SE Vector Knob"),
	Register<VectorPanKnob>::withId(L"SE Vector Pan Knob"),
	Register<VectorKnob_VCV>::withId(L"SE Vector Knob2"),
	};
}
