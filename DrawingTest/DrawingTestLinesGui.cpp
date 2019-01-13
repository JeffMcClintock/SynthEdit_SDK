#include "./DrawingTestLinesGui.h"
#include "Drawing.h"

using namespace gmpi;
using namespace GmpiDrawing;

GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, DrawingTestLinesGui, L"SE Drawing Test - Lines" );

DrawingTestLinesGui::DrawingTestLinesGui()
{
	// initialise pins.
	initializePin( pinTest, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestLinesGui::onSetTest) );
	initializePin( pinFontface, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestLinesGui::onSetFontface) );
	initializePin( pinFontsize, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestLinesGui::onSetFontsize) );
	initializePin( pinText, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestLinesGui::onSetText) );
	initializePin( pinGammaCorrection, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestLinesGui::onSetGammaCorrection) );
	initializePin( pinAdjust, static_cast<MpGuiBaseMemberPtr2>(&DrawingTestLinesGui::onSetAdjust) );
}

void line(int x0, int y0, int x1, int y1, int32_t* image, int stride, int32_t color) {
	bool steep = false;
	if (std::abs(x0 - x1)<std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0>x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (steep) {
//			image.set(y, x, color);
			image[x * stride + y] = color;
		}
		else {
//			image.set(x, y, color);
			image[y * stride + x] = color;
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y1>y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

int32_t DrawingTestLinesGui::OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext )
{
	Graphics g(drawingContext);
	auto r = getRect();

	if (backbuffer.isNull())
	{
		backbuffer = g.CreateCompatibleRenderTarget(Size(r.getWidth(), r.getHeight()));
		backbuffer2 = g.GetFactory().CreateImage((int32_t)r.getWidth(), (int32_t)r.getHeight());
	}

	{
		auto pixels = backbuffer2.lockPixels();
		auto imageSize = backbuffer2.GetSize();
		int totalPixels = (int)imageSize.height * pixels.getBytesPerRow() / sizeof(uint32_t);

		int32_t* sourcePixels = (int32_t*) pixels.getAddress();

		static int32_t col = 0;
		//for (int i = 0; i < totalPixels; ++i)
		//{
		//	++col;
		//	*sourcePixels++ = col | 0xFF000000;
		//}

		int w = (int)r.getWidth();
		int h = (int)r.getHeight();
		for (int x = w; x > 0; --x)
		{
			line(0, 0, x, h - 1, sourcePixels, pixels.getBytesPerRow() / sizeof(uint32_t), ++col | 0xFF000000);
		}
	}
/*
	backbuffer.BeginDraw();
	backbuffer.Clear(Color::BlueViolet);//  Color((uint32_t)0, 0.0f));

	backbuffer.EndDraw();
	g.DrawBitmap(backbuffer.GetBitmap(), Point(0, 0), r);
*/

	g.DrawBitmap(backbuffer2, Point(0, 0), r);
	
	return gmpi::MP_OK;
}

int32_t DrawingTestLinesGui::initialize()
{
	auto r = gmpi_gui::MpGuiGfxBase::initialize(); // ensure all pins initialised (so widgets are built).

	StartTimer(16);

	return r;
}

bool DrawingTestLinesGui::OnTimer()
{
	invalidateRect();
	return true;
}

// handle pin updates.
void DrawingTestLinesGui::onSetTest()
{
	// pinTest changed
}

void DrawingTestLinesGui::onSetFontface()
{
	// pinFontface changed
}

void DrawingTestLinesGui::onSetFontsize()
{
	// pinFontsize changed
}

void DrawingTestLinesGui::onSetText()
{
	invalidateRect();
}

void DrawingTestLinesGui::onSetGammaCorrection()
{
	// pinGammaCorrection changed
}

void DrawingTestLinesGui::onSetAdjust()
{
	invalidateRect();
}