#include "pch.h"
#include <d2d1_2.h>
#include <d3d11_1.h>
#include <wrl.h> // Comptr
#include "./DrawingFrame_win32.h"
#include "../shared/it_enum_list.h"
#include "../shared/xp_dynamic_linking.h"
#include "../shared/xp_simd.h"
#include <Windowsx.h>
#include "IGuiHost.h"
#include <commctrl.h>

#ifdef _WIN32

//#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#if defined(SE_EDIT_SUPPORT) || defined(SE_TARGET_VST2) || defined(SE_TARGET_VST3)
#include "resource.h"
#endif
#include <Commdlg.h>

#endif

using namespace std;
using namespace gmpi;
using namespace gmpi_gui;
using namespace GmpiGuiHosting;
using namespace GmpiDrawing_API;

using namespace Microsoft::WRL;
using namespace D2D1;

// WIN32 Edit box dialog.
#ifdef _WIN32

//#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#if defined(SE_EDIT_SUPPORT) || defined(SE_TARGET_VST2) || defined(SE_TARGET_VST3)
namespace GmpiGuiHosting
{

LRESULT CALLBACK DrawingFrameWindowProc(HWND hwnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	auto drawingFrame = (DrawingFrame*)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (drawingFrame)
	{
		return drawingFrame->WindowProc(message, wParam, lParam);
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

// copied from MP_GetDllHandle
HMODULE local_GetDllHandle_randomshit()
{
	HMODULE hmodule = 0;
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&local_GetDllHandle_randomshit, &hmodule);
	return (HMODULE)hmodule;
}

bool registeredWindowClass = false;
WNDCLASS windowClass;
wchar_t gClassName[100];

void DrawingFrame::open(void* pParentWnd)
{
	parentWnd = (HWND)pParentWnd;

	RECT r;
//	GetWindowRect(parentWnd, &r);// find parents absolute location and size.
	GetClientRect(parentWnd, &r);

	if (!registeredWindowClass)
	{
		registeredWindowClass = true;
		OleInitialize(0);

		swprintf(gClassName, L"GMPIGUI%p", local_GetDllHandle_randomshit());

		windowClass.style = CS_GLOBALCLASS;// | CS_DBLCLKS;//|CS_OWNDC; // add Private-DC constant 

		windowClass.lpfnWndProc = DrawingFrameWindowProc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = local_GetDllHandle_randomshit();
		windowClass.hIcon = 0;

		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
#if DEBUG_DRAWING
		windowClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
#else
		windowClass.hbrBackground = 0;
#endif
		windowClass.lpszMenuName = 0;
		windowClass.lpszClassName = gClassName;
		RegisterClass(&windowClass);

		//		bSwapped_mouse_buttons = GetSystemMetrics(SM_SWAPBUTTON) > 0;
	}

	int style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;// | WS_OVERLAPPEDWINDOW;
	int extended_style = 0;

	windowHandle = CreateWindowEx(extended_style, gClassName, L"",
		style, 0, 0, r.right - r.left, r.bottom - r.top,
		parentWnd, NULL, local_GetDllHandle_randomshit(), NULL);

	if (windowHandle)
	{
		SetWindowLongPtr(windowHandle, GWLP_USERDATA, (__int3264)(LONG_PTR)this);
		//		RegisterDragDrop(windowHandle, new CDropTarget(this));

		D2D1_SIZE_U size = D2D1::SizeU(
			r.right - r.left,
			r.bottom - r.top
		);

		CreateRenderTarget(size);

		initTooltip();
	}
}

void DrawingFrameBase::initTooltip()
{
	if (tooltipWindow == nullptr && getWindowHandle())
	{
		auto instanceHandle = local_GetDllHandle_randomshit();
		{
			TOOLINFO ti;
			// Create the ToolTip control.
			HWND hwndTT = CreateWindow(TOOLTIPS_CLASS, TEXT(""),
				WS_POPUP,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				NULL, (HMENU)NULL, instanceHandle,
				NULL);

			// Prepare TOOLINFO structure for use as tracking ToolTip.
			ti.cbSize = sizeof(TOOLINFO);
			ti.uFlags = TTF_SUBCLASS;
			ti.hwnd = (HWND)getWindowHandle();
			ti.uId = (UINT)0;
			ti.hinst = instanceHandle;
			ti.lpszText = const_cast<TCHAR*> (TEXT("This is a tooltip"));
			ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0;

			// Add the tool to the control
			if (!SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti))
			{
				DestroyWindow(hwndTT);
				return;
			}

			tooltipWindow = hwndTT;
		}
	}
}

void DrawingFrameBase::TooltipOnMouseActivity()
{
	if(toolTipShown)
	{
		if (toolTiptimer < -20) // ignore spurious MouseMove when Tooltip shows
		{
			HideToolTip();
			toolTiptimer = toolTiptimerInit;
		}
	}
	else
		toolTiptimer = toolTiptimerInit;
}

void DrawingFrameBase::ShowToolTip()
{
	_RPT0(_CRT_WARN, "YEAH!\n");

	//UTF8StringHelper tooltipText(tooltip);
	//if (platformObject)
	{
		auto platformObject = tooltipWindow;

		RECT rc;
		rc.left = (LONG)0;
		rc.top = (LONG)0;
		rc.right = (LONG)100000;
		rc.bottom = (LONG)100000;
		TOOLINFO ti = { 0 };
		ti.cbSize = sizeof(TOOLINFO);
		ti.hwnd = (HWND)getWindowHandle(); // frame->getSystemWindow();
		ti.uId = 0;
		ti.rect = rc;
		ti.lpszText = (TCHAR*)(const TCHAR*)toolTipText.c_str();
		SendMessage((HWND)platformObject, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		SendMessage((HWND)platformObject, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);
		SendMessage((HWND)platformObject, TTM_POPUP, 0, 0);
	}
//#endif // WINDOWS
	toolTipShown = true;
}

void DrawingFrameBase::HideToolTip()
{
	toolTipShown = false;
	_RPT0(_CRT_WARN, "NUH!\n");

	if (tooltipWindow)
	{
		TOOLINFO ti = { 0 };
		ti.cbSize = sizeof(TOOLINFO);
		ti.hwnd = (HWND)getWindowHandle(); // frame->getSystemWindow();
		ti.uId = 0;
		ti.lpszText = 0;
		SendMessage((HWND)tooltipWindow, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		SendMessage((HWND)tooltipWindow, TTM_POP, 0, 0);
	}
}

LRESULT DrawingFrame::WindowProc(UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (message)
	{
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			GmpiDrawing::Point p(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			p = WindowToDips.TransformPoint(p);

			// Cubase sends spurious mouse move messages when transport running.
			// This prevents tooltips working.
			if (message == WM_MOUSEMOVE)
			{
				if (cubaseBugPreviousMouseMove == p)
				{
					return TRUE;
				}
				cubaseBugPreviousMouseMove = p;
			}
			else
			{
				cubaseBugPreviousMouseMove = GmpiDrawing::Point(-1, -1);
			}

			TooltipOnMouseActivity();

			int32_t flags = gmpi_gui_api::GG_POINTER_FLAG_INCONTACT | gmpi_gui_api::GG_POINTER_FLAG_PRIMARY | gmpi_gui_api::GG_POINTER_FLAG_CONFIDENCE;

			switch (message)
			{
				case WM_MBUTTONDOWN:
				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
					flags |= gmpi_gui_api::GG_POINTER_FLAG_NEW;
					break;
			}

			switch (message)
			{
			case WM_LBUTTONUP:
			case WM_LBUTTONDOWN:
				flags |= gmpi_gui_api::GG_POINTER_FLAG_FIRSTBUTTON;
				break;
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				flags |= gmpi_gui_api::GG_POINTER_FLAG_SECONDBUTTON;
				break;
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
				flags |= gmpi_gui_api::GG_POINTER_FLAG_THIRDBUTTON;
				break;
			}

			if (GetKeyState(VK_SHIFT) < 0)
			{
				flags |= gmpi_gui_api::GG_POINTER_KEY_SHIFT;
			}
			if (GetKeyState(VK_CONTROL) < 0)
			{
				flags |= gmpi_gui_api::GG_POINTER_KEY_CONTROL;
			}
			if (GetKeyState(VK_MENU) < 0)
			{
				flags |= gmpi_gui_api::GG_POINTER_KEY_ALT;
			}

			int32_t r;
			switch (message)
			{
			case WM_MOUSEMOVE:
				{
					r = containerView->onPointerMove(flags, p);
				}
				break;

			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
				r = containerView->onPointerDown(flags, p);
				break;

			case WM_MBUTTONUP:
			case WM_RBUTTONUP:
			case WM_LBUTTONUP:
				r = containerView->onPointerUp(flags, p);
				break;
			}
		}
		break;

	case WM_NCACTIVATE:
		//if( wParam == FALSE ) // USER CLICKED AWAY
		//	goto we_re_done;
		break;

	case WM_WINDOWPOSCHANGING:
	{
		LPWINDOWPOS wp = (LPWINDOWPOS)lParam;
		int test = wp->cx;
	}
	break;

	case WM_ACTIVATE:
	{
		/*
		//HFONT hFont = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		//	CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, TEXT("Courier New"));

		SendMessage(child,      // Handle of edit control
		WM_SETFONT,         // Message to change the font
		(WPARAM)dialogFont,      // handle of the font
		MAKELPARAM(TRUE, 0) // Redraw text
		);

		::SetWindowPos(hwndDlg, 0, dialogX, dialogY, dialogW, dialogH, SWP_NOZORDER);
		::SetWindowPos(child, 0, 0, 0, dialogW, dialogH, SWP_NOZORDER);
		::SetWindowText(child, dialogEditText);
		::SetFocus(child);
		dialogReturnValue = 1;
		// Select all.
		#ifdef WIN32
		SendMessage(child, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		#else
		SendMessage(child, EM_SETSEL, 0, MAKELONG(0, -1));
		#endif
		*/
	}
	break;

	case WM_COMMAND:
		/*
		switch( LOWORD(wParam) )
		{
		case IDOK:
		goto we_re_done;
		break;

		case IDCANCEL:
		dialogReturnValue = 0;
		EndDialog(hwndDlg, dialogReturnValue); // seems to call back here and exit at "we_re_done"
		return TRUE;
		}
		*/
		break;

	case WM_PAINT:
	{
		OnPaint();
		::DefWindowProc(windowHandle, message, wParam, lParam); // clear update rect.
	}
	break;

	default:
		return DefWindowProc(windowHandle, message, wParam, lParam);

	//we_re_done:
	//	if( !GetDlgItemText(hwndDlg, IDC_EDIT1, dialogEditText, sizeof(dialogEditText) / sizeof(dialogEditText[0])) )
	//		*dialogEditText = 0;
	//	EndDialog(hwndDlg, dialogReturnValue);

	}
	return TRUE;
}

void DrawingFrameBase::Init(SynthEdit2::IPresenter* presenter, int pviewType)
{
	AddView(new SynthEdit2::ContainerView());
	GmpiDrawing::Rect viewRect{ 0, 0, static_cast<float>(viewDimensions), static_cast<float>(viewDimensions) };
	containerView->arrange(viewRect);
	containerView->setDocument(presenter, pviewType);
}

// Ideally this is called at 60Hz so we can draw as fast as practical, but without blocking to wait for Vsync all the time (makes host unresponsive).
bool DrawingFrameBase::OnTimer()
{
	auto hwnd = getWindowHandle();
	if (hwnd == nullptr)
		return true;

	// Tooltips
	if (toolTiptimer-- < 0 && !toolTipShown)
	{
		POINT P;
		GetCursorPos(&P);

		// Check mouse in window and not captured.
		if (WindowFromPoint(P) == hwnd && GetCapture() != hwnd)
		{
			ScreenToClient(hwnd, &P);
			//			GmpiDrawing::Point point(P.x, P.y);

			auto point = WindowToDips.TransformPoint(GmpiDrawing::Point(P.x, P.y));

			// get item under mouse.
			// toolTipText = L"Doog";
			auto text = containerView->getToolTip(point);
			if (!text.empty())
			{
				toolTipText = Utf8ToWstring(text);
				ShowToolTip();
			}
		}
	}
	
	// Get any meter updates from DSP. ( See also CSynthEditAppBase::OnTimer() )
//	controller->serviceGuiQueue();
	containerView->Presenter()->GetPatchManager()->serviceGuiQueue();

	// Queue pending drawing updates to backbuffer.
	const BOOL bErase = FALSE;

	for (auto& invalidRect : backBufferDirtyRects)
	{
		::InvalidateRect(hwnd, reinterpret_cast<RECT*>(&invalidRect), bErase);
	}
	backBufferDirtyRects.clear();
#if 0 // force redraw.
	RECT test;
	test.top = test.left = 0;
	test.right = test.bottom = 1;
	::InvalidateRect(hwnd, reinterpret_cast<RECT*>(&test), bErase);
#endif
	return true;
}

void DrawingFrameBase::OnPaint()
{
	// First clear update region (else windows will pound on this repeatedly).
	UpdateRegionWinGdi updateRegion(getWindowHandle());
	ValidateRect(getWindowHandle(), NULL); // Clear invalid region for next frame.

	// prevent infinite assert dialog boxes when assert happens during painting.
	if (reentrant)
	{
		return;
	}
	reentrant = true;

	auto dirtyRects = updateRegion.getUpdateRects();
	if (containerView && !dirtyRects.empty())
	{
		//	_RPT1(_CRT_WARN, "OnPaint(); %d dirtyRects\n", dirtyRects.size() );

		if (!mpRenderTarget) // not quite right, also need to re-create any resources (brushes etc) else most object draw blank. Could refresh the view in this case.
		{
			CreateDevice();
		}

		std::unique_ptr<gmpi::directx::GraphicsContext> context;
		if (DX_support_sRGB)
		{
			context.reset(new gmpi::directx::GraphicsContext(mpRenderTarget, &DrawingFactory));
		}
		else
		{
			context.reset(new gmpi::directx::GraphicsContext_Win7(mpRenderTarget, &DrawingFactory));
		}

		context->BeginDraw();

		GmpiDrawing::Graphics graphics(context.get());

		graphics.SetTransform(viewTransform);

#if 1
		for (auto& r : dirtyRects)
		{
			auto r2 = WindowToDips.TransformRect(GmpiDrawing::Rect(r.left, r.top, r.right, r.bottom));

			// Snap to whole DIPs.
			GmpiDrawing::Rect temp;
			temp.left = static_cast<float>(FastRealToIntTruncateTowardZero(r2.left));
			temp.top = static_cast<float>(FastRealToIntTruncateTowardZero(r2.top));
			temp.right = static_cast<float>(FastRealToIntTruncateTowardZero(r2.right) + 1);
			temp.bottom = static_cast<float>(FastRealToIntTruncateTowardZero(r2.bottom) + 1);

			graphics.PushAxisAlignedClip(temp);

			containerView->OnRender(static_cast<GmpiDrawing_API::IMpDeviceContext*>(context.get()));
			graphics.PopAxisAlignedClip();
		}

#else
		// Clip to Windows update rect, for efficiency.
		RECT rect;
		BOOL r = GetUpdateRect(getWindowHandle(), &rect, FALSE);
		//	_RPTW4(_CRT_WARN, L"OnPaint() (%d,%d,%d,%d) \n", rect.left, rect.top, rect.right, rect.bottom);
		if (r != 0)
		{
			GmpiDrawing::Rect cliprect(rect.left, rect.top, rect.right, rect.bottom);
			graphics.PushAxisAlignedClip(cliprect);
		}

		containerView->OnRender(static_cast<GmpiDrawing_API::IMpDeviceContext*>(&context));

		if (r != 0)
			graphics.PopAxisAlignedClip();
#endif
		// Print OS Version.
#if 0 //def _DEBUG
		{
			OSVERSIONINFO osvi;
			memset(&osvi, 0, sizeof(osvi));

			osvi.dwOSVersionInfoSize = sizeof(osvi);
			GetVersionEx(&osvi);

			char versionString[100];
			sprintf(versionString, "OS Version %d.%d", (int)osvi.dwMajorVersion, (int)osvi.dwMinorVersion);

			auto brsh = graphics.CreateSolidColorBrush(GmpiDrawing::Color::Black);
			graphics.DrawTextU(versionString, graphics.GetFactory().CreateTextFormat(12), GmpiDrawing::Rect(2.0f, 2.0f, 200, 200), brsh);
		}
#endif

		// Print Frame Rate
//		const bool displayFrameRate = true;
		const bool displayFrameRate = false;
//		static int presentTimeMs = 0;
		if(displayFrameRate)
		{
			static int frameCount = 0;
			static char frameCountString[100] = "";
			if (++frameCount == 60)
			{
				auto timenow = std::chrono::steady_clock::now();
				auto elapsed = std::chrono::steady_clock::now() - frameCountTime;
				auto elapsedSeconds = 0.001f * (float)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

				float frameRate = frameCount / elapsedSeconds;

//				sprintf(frameCountString, "%3.1f FPS. %dms PT", frameRate, presentTimeMs);
				sprintf(frameCountString, "%3.1f FPS", frameRate);
				frameCountTime = timenow;
				frameCount = 0;

				auto brush = graphics.CreateSolidColorBrush(GmpiDrawing::Color::Black);
				auto fpsRect = GmpiDrawing::Rect(0, 0, 50, 18);
				graphics.FillRectangle(fpsRect, brush);
				brush.SetColor(GmpiDrawing::Color::White);
				graphics.DrawTextU(frameCountString, graphics.GetFactory().CreateTextFormat(12), fpsRect, brush);

				dirtyRects.push_back(GmpiDrawing::RectL(0, 0, 100, 36));
			}
		}

		auto hr = context->EndDraw();

		//	frontBufferDirtyRects.insert(frontBufferDirtyRects.end(), dirtyRects.begin(), dirtyRects.end());

		// Present the backbuffer (if it has some new content)
		{
			HRESULT hr = S_OK;

			// Can't use dirty rects on fresh backbuffer until present called at least once (successfully).
			// MOved to : DrawingFrameBase::CreateDeviceSwapChainBitmap()
			/*
			if (frontBufferNew)
			{
				_RPT0(_CRT_WARN, "Present(0, 0);\n");

				hr = m_swapChain->Present(0, 0);

				if (S_OK == hr || DXGI_STATUS_OCCLUDED == hr)
					frontBufferNew = false;

				InvalidateRect(getWindowHandle(), NULL, FALSE); // Any drawing happening before this initial Present() has no effect, need to ensure entire window is drawn fresh.
			}
			else
			*/
			{
				assert(!dirtyRects.empty());

				DXGI_PRESENT_PARAMETERS presetParameters;
				presetParameters.pScrollRect = nullptr;
				presetParameters.pScrollOffset = nullptr;
				presetParameters.DirtyRectsCount = dirtyRects.size();
				presetParameters.pDirtyRects = reinterpret_cast<RECT*>(dirtyRects.data()); // should be exact same layout.

				// checkout DXGI_PRESENT_DO_NOT_WAIT
//				hr = m_swapChain->Present1(1, DXGI_PRESENT_TEST, &presetParameters);
//				_RPT1(_CRT_WARN, "Present1() test = %x\n", hr);
/* NEVER returns DXGI_ERROR_WAS_STILL_DRAWING
	//			_RPT1(_CRT_WARN, "Present1() DirtyRectsCount = %d\n", presetParameters.DirtyRectsCount);
				hr = m_swapChain->Present1(1, DXGI_PRESENT_DO_NOT_WAIT, &presetParameters);
				if (hr == DXGI_ERROR_WAS_STILL_DRAWING)
				{
					_RPT1(_CRT_WARN, "Present1() Blocked\n", hr);
*/
					// Present(0... improves framerate only from 60 -> 64 FPS, so must be blocking a little with "1".
//				auto timeA = std::chrono::steady_clock::now();
				hr = m_swapChain->Present1(1, 0, &presetParameters);
				//auto elapsed = std::chrono::steady_clock::now() - timeA;
				//presentTimeMs = (float)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
//				}
/* could put this in timer to reduce blocking, agregating dirty rects until call successful.
*/
			}

			if (S_OK != hr && DXGI_STATUS_OCCLUDED != hr)
			{
				// DXGI_ERROR_INVALID_CALL 0x887A0001L
				ReleaseDevice();
			}
		}
	}

	reentrant = false;
}

void DrawingFrameBase::CreateDevice()
{
	ReleaseDevice();

	// Create a Direct3D Device
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
// you must explicity install DX debug support for this to work.
//	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// Comment out first to test lower versions.
	D3D_FEATURE_LEVEL d3dLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	D3D_FEATURE_LEVEL currentDxFeatureLevel;
	ComPtr<ID3D11Device> D3D11Device;
	
	HRESULT r = DXGI_ERROR_UNSUPPORTED;
#if 1 //ndef _DEBUG
	r = D3D11CreateDevice(nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		d3dLevels, sizeof(d3dLevels) / sizeof(d3dLevels[0]),
		D3D11_SDK_VERSION,
		D3D11Device.GetAddressOf(),
		&currentDxFeatureLevel,
		nullptr);
#endif
	if (DXGI_ERROR_UNSUPPORTED == r)
	{
		r = D3D11CreateDevice(nullptr,
			D3D_DRIVER_TYPE_WARP,
			nullptr,
			flags,
			nullptr, 0,
			D3D11_SDK_VERSION,
			D3D11Device.GetAddressOf(),
			&currentDxFeatureLevel,
			nullptr);
	}

	// query for the device object’s IDXGIDevice interface
	ComPtr<IDXGIDevice> dxdevice;
	D3D11Device.As(&dxdevice);

	// Retrieve the display adapter
	ComPtr<IDXGIAdapter> adapter;
	dxdevice->GetAdapter(adapter.GetAddressOf());

	// adapter’s parent object is the DXGI factory
	ComPtr<IDXGIFactory2> factory;
	adapter->GetParent(__uuidof(factory), reinterpret_cast<void **>(factory.GetAddressOf()));

	{
		OSVERSIONINFO osvi;
		memset(&osvi, 0, sizeof(osvi));

		osvi.dwOSVersionInfoSize = sizeof(osvi);
		GetVersionEx(&osvi);

		DX_support_sRGB =
			((osvi.dwMajorVersion > 6) ||
			((osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion > 1))); // Win7 = V6.1
	}

	DX_support_sRGB &= D3D_FEATURE_LEVEL_11_0 <= currentDxFeatureLevel;

#if 0 //def _DEBUG
	DX_support_sRGB = false;
#endif

	DrawingFactory.setSrgbSupport(DX_support_sRGB);

	// Create swap-chain.
	DXGI_SWAP_CHAIN_DESC1 props = {};
	if (DX_support_sRGB)
	{
		props.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // Proper gamma-correct blending.
	}
	else
	{
		props.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // shitty linear blending.
	}

	props.SampleDesc.Count = 1;
	props.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	props.BufferCount = 2;
	props.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

	factory->CreateSwapChainForHwnd(D3D11Device.Get(),
		getWindowHandle(),
		&props,
		nullptr,
		nullptr,
		&m_swapChain);

	// Creating the Direct2D Device
	ComPtr<ID2D1Device> device;
	DrawingFactory.getD2dFactory()->CreateDevice(dxdevice.Get(),
		device.GetAddressOf());

	// and context.
	device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &mpRenderTarget);

	float dpiX, dpiY;
	DrawingFactory.getD2dFactory()->GetDesktopDpi(&dpiX, &dpiY);

#if 0 //def _DEBUG
	// disable DPI for testing.
	dpiX = dpiY = 96.0f;
#endif

	mpRenderTarget->SetDpi(dpiX, dpiY);

	/*
	dpiScale.x = 96.f / dpiX;
	dpiScale.y = 96.f / dpiY;

	dpiScaleInverse.x = dpiX / 96.f;
	dpiScaleInverse.y = dpiY / 96.f;
	 */

	// For windows messages, include DPI
	/*
	 *
	HDC hdc = ::GetDC(m_hWnd);
	int lx = GetDeviceCaps(hdc, LOGPIXELSX);
	int ly = GetDeviceCaps(hdc, LOGPIXELSY);
	::ReleaseDC(m_hWnd, hdc);
	 */
	DipsToWindow = GmpiDrawing::Matrix3x2::Scale(dpiX / 96.0f, dpiY / 96.0f); // was dpiScaleInverse
	WindowToDips = DipsToWindow;
	WindowToDips.Invert();

	mpRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE); // "The quality of rendering grayscale text is comparable to ClearType but is much faster."}

	CreateDeviceSwapChainBitmap();
}

void DrawingFrameBase::CreateDeviceSwapChainBitmap()
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_RPT0(_CRT_WARN, "\n\nCreateDeviceSwapChainBitmap()\n");

	ComPtr<IDXGISurface> surface;
	m_swapChain->GetBuffer(0, // buffer index
		__uuidof(surface),
		reinterpret_cast<void **>(surface.GetAddressOf()));

	// Get the swapchain pixel format.
	DXGI_SURFACE_DESC sufaceDesc;
	surface->GetDesc(&sufaceDesc);

	auto props2 = BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		PixelFormat(sufaceDesc.Format, D2D1_ALPHA_MODE_IGNORE)
		);

	ComPtr<ID2D1Bitmap1> bitmap;
	mpRenderTarget->CreateBitmapFromDxgiSurface(surface.Get(),
		props2,
		bitmap.GetAddressOf());

	// Now attach Device Context to swapchain bitmap.
	mpRenderTarget->SetTarget(bitmap.Get());

//	frontBufferNew = true;
	// Initial present() moved here in order to ensure it happens before first Timer() tries to draw anything.
	{
		auto hr = m_swapChain->Present(0, 0);
//		frontBufferNew = false;
	}

	InvalidateRect(getWindowHandle(), nullptr, false);
}

void DrawingFrameBase::CreateRenderTarget(D2D1_SIZE_U& size)
{
	CreateDevice();

	StartTimer(15); // 16.66 = 60Hz. 16ms timer seems to miss v-sync. Faster timers offer no improvement to framerate.
}

// Convert to an integer rect, ensuring it surrounds all partial pixels.
inline GmpiDrawing::RectL RectToIntegerLarger(GmpiDrawing_API::MP1_RECT f)
{
	GmpiDrawing::RectL r;
	r.left = FastRealToIntTruncateTowardZero(f.left);
	r.top = FastRealToIntTruncateTowardZero(f.top);
	r.right = FastRealToIntTruncateTowardZero(f.right) + 1;
	r.bottom = FastRealToIntTruncateTowardZero(f.bottom) + 1;

	return r;
}

void MP_STDCALL DrawingFrameBase::invalidateRect(const GmpiDrawing_API::MP1_RECT* invalidRect)
{
	GmpiDrawing::RectL r;
	if (invalidRect)
	{
		/*
		//_RPT4(_CRT_WARN, "invalidateRect r[ %d %d %d %d]\n", (int)invalidRect->left, (int)invalidRect->top, (int)invalidRect->right, (int)invalidRect->bottom);

		// Transform to desktop pixels.
		auto leftTop = view Transform.TransformPoint(GmpiDrawing::Point(invalidRect->left, invalidRect->top));
		auto rightBottom = view Transform.TransformPoint(GmpiDrawing::Point(invalidRect->right, invalidRect->bottom));

		r.left = FastRealToIntTruncateTowardZero(leftTop.x * dpi Scale Inverse.x);
		r.top = FastRealToIntTruncateTowardZero(leftTop.y * dpi Scale Inverse.x);
		r.right = FastRealToIntTruncateTowardZero(rightBottom.x * dpi Scale Inverse.x) + 1;
		r.bottom = FastRealToIntTruncateTowardZero(rightBottom.y * dpi Scale Inverse.x) + 1;
		*/

		r = RectToIntegerLarger( DipsToWindow.TransformRect(*invalidRect) );
	}
	else
	{
		GetClientRect(getWindowHandle(), reinterpret_cast<RECT*>( &r ));
	}

	auto area1 = r.getWidth() * r.getHeight();

	for (auto& dirtyRect : backBufferDirtyRects )
	{
		auto area2 = dirtyRect.getWidth() * dirtyRect.getHeight();

		GmpiDrawing::RectL unionrect(dirtyRect);

		unionrect.top = (std::min)(unionrect.top, r.top);
		unionrect.bottom = (std::max)(unionrect.bottom, r.bottom);
		unionrect.left = (std::min)(unionrect.left, r.left);
		unionrect.right = (std::max)(unionrect.right, r.right);

		auto unionarea = unionrect.getWidth() * unionrect.getHeight();

		if (unionarea <= area1 + area2)
		{
			// replace existing rect with combined rect
			dirtyRect = unionrect;
			return;
			break;
		}
	}

	// no optimisation found, add new rect.
	backBufferDirtyRects.push_back(r);
}

void MP_STDCALL DrawingFrameBase::invalidateMeasure()
{
}

int32_t DrawingFrameBase::setCapture(void)
{
	::SetCapture(getWindowHandle());
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::getCapture(int32_t & returnValue)
{
	returnValue = ::GetCapture() == getWindowHandle();
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::releaseCapture(void)
{
	::ReleaseCapture();

	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::createPlatformMenu(GmpiDrawing_API::MP1_RECT* rect, gmpi_gui::IMpPlatformMenu** returnMenu)
{
	auto nativeRect = DipsToWindow.TransformRect(*rect);
	*returnMenu = new GmpiGuiHosting::PGCC_PlatformMenu(getWindowHandle(), &nativeRect, DipsToWindow._22);
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::createPlatformTextEdit(GmpiDrawing_API::MP1_RECT* rect, gmpi_gui::IMpPlatformText** returnTextEdit)
{
#ifdef SE_TARGET_WINDOWS_STORE_APP
	*returnTextEdit = new PlatformTextEntry(this);
#else
	auto nativeRect = DipsToWindow.TransformRect(*rect);
	*returnTextEdit = new GmpiGuiHosting::PGCC_PlatformTextEntry(getWindowHandle(), &nativeRect, DipsToWindow._22);
#endif

	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::createFileDialog(int32_t dialogType, gmpi_gui::IMpFileDialog** returnFileDialog)
{
	*returnFileDialog = new Gmpi_Win_FileDialog(dialogType, getWindowHandle());
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::createOkCancelDialog(int32_t dialogType, gmpi_gui::IMpOkCancelDialog** returnDialog)
{
	*returnDialog = new Gmpi_Win_OkCancelDialog(dialogType, getWindowHandle());
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::pinTransmit(int32_t pinId, int32_t size, const void * data, int32_t voice)
{
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::createPinIterator(gmpi::IMpPinIterator** returnIterator)
{
	return gmpi::MP_FAIL;
}

int32_t DrawingFrameBase::getHandle(int32_t & returnValue)
{
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::sendMessageToAudio(int32_t id, int32_t size, const void * messageData)
{
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::ClearResourceUris()
{
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::RegisterResourceUri(const char * resourceName, const char * resourceType, gmpi::IString* returnString)
{
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::OpenUri(const char * fullUri, gmpi::IProtectedFile2** returnStream)
{
	return gmpi::MP_OK;
}

int32_t DrawingFrameBase::FindResourceU(const char * resourceName, const char * resourceType, gmpi::IString* returnString)
{
	return gmpi::MP_OK;
}

} //namespace

#endif // desktop

#else //WIN32

//int32_t PGCC_PlatformTextEntry::Show(float x, float y, float w, float h, IMpUnknown* returnString)
int32_t PGCC_PlatformTextEntry::ShowAsync(IMpUgmpi_gui::ICompletionCallbacknknown* returnCompletionHandler)
{
	gmpi_gui::ICompletionCallback* callback;
	if (MP_OK != returnCompletionHandler->queryInterface(gmpi_gui::SE_IID_COMPLETION_CALLBACK, reinterpret_cast<void**>( &callback)) )
	{
		return gmpi::MP_NOSUPPORT;
	}

	int out_dismissedKey;
//	ActivateTextBoxV(parentWnd, offsetX + x, offsetY + y, w, h, text_, &out_dismissedKey);
	ActivateTextBoxV(parentWnd, offsetX + editrect.left, offsetY + editrect.top, editrect.getWidth(), editrect.getHeight(), text_, &out_dismissedKey);

	bool canceled = false; // out_dismissedKey == ??
//	if (!canceled)
//	{
//		text_ = WStringToUtf8(dialogEditText);
//	}

	callback->OnComplete(!canceled ? gmpi::MP_OK : gmpi::MP_CANCEL);

	return gmpi::MP_OK;
}

#endif