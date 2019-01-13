#pragma once

/*
#include "modules/se_sdk3_hosting/DrawingFrame_win32.h"
using namespace GmpiGuiHosting;
*/

#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <d3d11_1.h>
#include "../Shared/ContainerView.h"
#include "TimerManager.h"

#ifdef _WIN32

#include "DirectXGfx.h"

#if defined(SE_EDIT_SUPPORT) || defined(SE_TARGET_VST2) || defined(SE_TARGET_VST3)
#include "../../conversion.h"
#endif
#else
#include "my_edit_box_Mac_Cocoa.h"
#endif

#include "../se_sdk3/mp_sdk_gui2.h"

namespace SynthEdit2
{
	class IPresenter;
}

namespace GmpiGuiHosting
{
    
#ifdef _WIN32

// This code is for Win32 desktop apps
//#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#if defined(SE_EDIT_SUPPORT2) || defined(SE_EDIT_SUPPORT) || defined(SE_TARGET_VST2) || defined(SE_TARGET_VST3)

	// Base class for DrawingFrame (VST3 Plugins) and MyFrameWndDirectX (SynthEdit 1.4+ Panel View).
	class DrawingFrameBase : public gmpi_gui::IMpGraphicsHost, public gmpi::IMpUserInterfaceHost2, public TimerClient
	{
		std::chrono::time_point<std::chrono::steady_clock> frameCountTime;

	protected:
		const static int viewDimensions = 7968; // DIPs (divisible by grids 60x60 + 2 24 pixel borders)

		ID2D1DeviceContext* mpRenderTarget;
		IDXGISwapChain1* m_swapChain;
		bool DX_support_sRGB;

		gmpi_sdk::mp_shared_ptr<SynthEdit2::ContainerView> containerView;

		// Paint() uses Direct-2d which block on vsync. Therefore all invalid rects should be applied in one "hit", else windows message queue chokes calling WM_PAINT repeately and blocking on every rect.
		std::vector<GmpiDrawing::RectL> backBufferDirtyRects;
		GmpiDrawing::Matrix3x2 viewTransform;
		GmpiDrawing::Matrix3x2 DipsToWindow;
		GmpiDrawing::Matrix3x2 WindowToDips;
		int toolTiptimer;
		bool toolTipShown;
		HWND tooltipWindow;
		static const int toolTiptimerInit = 40; // x/60 Hz
		std::wstring toolTipText;
		bool reentrant;

	public:
		gmpi::directx::Factory DrawingFactory;

		DrawingFrameBase() :
			 containerView(0)
			,mpRenderTarget(nullptr)
			,m_swapChain(nullptr)
			, toolTipShown(false)
			, tooltipWindow(nullptr)
			, DX_support_sRGB(true)
			, reentrant(false)
		{
			DrawingFactory.Init();
		}

		virtual ~DrawingFrameBase()
		{
			StopTimer();

			// Free GUI objects first so they can release fonts etc before releasing factorys.
			containerView = nullptr;

			ReleaseDevice();
		}

		// to help re-create device when lost.
		void ReleaseDevice()
		{
			if (mpRenderTarget)
				mpRenderTarget->Release();
			if (m_swapChain)
				m_swapChain->Release();

			mpRenderTarget = nullptr;
			m_swapChain = nullptr;
		}

		void ResizeSwapChainBitmap()
		{
			mpRenderTarget->SetTarget(nullptr);
			if (S_OK == m_swapChain->ResizeBuffers(0,
				0, 0,
				DXGI_FORMAT_UNKNOWN,
				0))
			{
				CreateDeviceSwapChainBitmap();
			}
			else
			{
				ReleaseDevice();
			}
		}

		void CreateDevice();
		void CreateDeviceSwapChainBitmap();

		void Init(SynthEdit2::IPresenter* presentor/*, IGuiHost2* hostPatchManager*/, int pviewType);

		void AddView(SynthEdit2::ContainerView* pcontainerView)
		{
			containerView.Attach(pcontainerView);
			containerView->setHost(static_cast<gmpi_gui::IMpGraphicsHost*>(this));
		}

		void OnPaint();
		virtual HWND getWindowHandle() = 0;
		void CreateRenderTarget(D2D1_SIZE_U& size);

		// Inherited via IMpUserInterfaceHost2
		virtual int32_t MP_STDCALL pinTransmit(int32_t pinId, int32_t size, const void * data, int32_t voice = 0) override;
		virtual int32_t MP_STDCALL createPinIterator(gmpi::IMpPinIterator** returnIterator) override;
		virtual int32_t MP_STDCALL getHandle(int32_t & returnValue) override;
		virtual int32_t MP_STDCALL sendMessageToAudio(int32_t id, int32_t size, const void * messageData) override;
		virtual int32_t MP_STDCALL ClearResourceUris() override;
		virtual int32_t MP_STDCALL RegisterResourceUri(const char * resourceName, const char * resourceType, gmpi::IString* returnString) override;
		virtual int32_t MP_STDCALL OpenUri(const char * fullUri, gmpi::IProtectedFile2** returnStream) override;
		virtual int32_t MP_STDCALL FindResourceU(const char * resourceName, const char * resourceType, gmpi::IString* returnString) override;
		int32_t MP_STDCALL LoadPresetFile(const char* presetFilePath) override
		{
//			Presenter()->LoadPresetFile(presetFilePath);
			return gmpi::MP_FAIL;
		}

		// IMpGraphicsHost
		virtual void MP_STDCALL invalidateRect(const GmpiDrawing_API::MP1_RECT * invalidRect) override;
		virtual void MP_STDCALL invalidateMeasure() override;
		virtual int32_t MP_STDCALL setCapture(void) override;
		virtual int32_t MP_STDCALL getCapture(int32_t & returnValue) override;
		virtual int32_t MP_STDCALL releaseCapture(void) override;
		virtual int32_t MP_STDCALL GetDrawingFactory(GmpiDrawing_API::IMpFactory ** returnFactory) override
		{
			*returnFactory = &DrawingFactory;
			return gmpi::MP_OK;
		}

		virtual int32_t MP_STDCALL createPlatformMenu(GmpiDrawing_API::MP1_RECT* rect, gmpi_gui::IMpPlatformMenu** returnMenu) override;
		virtual int32_t MP_STDCALL createPlatformTextEdit(GmpiDrawing_API::MP1_RECT* rect, gmpi_gui::IMpPlatformText** returnTextEdit) override;
		virtual int32_t MP_STDCALL createFileDialog(int32_t dialogType, gmpi_gui::IMpFileDialog** returnFileDialog) override;
		virtual int32_t MP_STDCALL createOkCancelDialog(int32_t dialogType, gmpi_gui::IMpOkCancelDialog** returnDialog) override;

		// IUnknown methods
		virtual int32_t MP_STDCALL queryInterface(const gmpi::MpGuid& iid, void** returnInterface)
		{
			if (iid == gmpi::MP_IID_UI_HOST2)
			{
				// important to cast to correct vtable (ug_plugin3 has 2 vtables) before reinterpret cast
				*returnInterface = reinterpret_cast<void*>(static_cast<IMpUserInterfaceHost2*>(this));
				addRef();
				return gmpi::MP_OK;
			}

			if (iid == gmpi_gui::SE_IID_GRAPHICS_HOST || iid == gmpi_gui::SE_IID_GRAPHICS_HOST_BASE || iid == gmpi::MP_IID_UNKNOWN)
			{
				// important to cast to correct vtable (ug_plugin3 has 2 vtables) before reinterpret cast
				*returnInterface = reinterpret_cast<void*>(static_cast<IMpGraphicsHost*>(this));
				addRef();
				return gmpi::MP_OK;
			}

			*returnInterface = 0;
			return gmpi::MP_NOSUPPORT;
		}

		void initTooltip();
		void TooltipOnMouseActivity();
		void ShowToolTip();
		void HideToolTip();
		virtual bool OnTimer() override;
		virtual void autoScrollStart() {};
		virtual void autoScrollStop() {};

		GMPI_REFCOUNT_NO_DELETE;
	};

	// This is used in VST3. Native HWND window frame.
	class DrawingFrame : public DrawingFrameBase
	{
		HWND windowHandle;
		HWND parentWnd;
		GmpiDrawing::Point cubaseBugPreviousMouseMove = { -1,-1 };

	public:
		DrawingFrame() :
			windowHandle(0)
		{
		}
		
		virtual HWND getWindowHandle() override
		{
			return windowHandle;
		}

		LRESULT WindowProc(UINT message, WPARAM wParam,	LPARAM lParam);
		void open(void* pParentWnd);
		virtual void DoClose() {};
	};

#endif


#else // Mac

    /*
class PGCC_PlatformTextEntry : public gmpi_gui::IMpPlatformText
{
	NSView* parentWnd;
//	int align;
	float dpiScale;
	int offsetX;
	int offsetY;

public:
	std::string text_;

	PGCC_PlatformTextEntry(void* pParentWnd, int poffsetX, int poffsetY, float dpi) :
		 parentWnd((NSView*) pParentWnd)
//		, align(TPM_LEFTALIGN)
		, dpiScale(dpi)
		, offsetX(poffsetX)
		, offsetY(poffsetY)
	{
		//		htextedit = CreatePopupMenu();
	}

	virtual int32_t MP_STDCALL SetText(const char* text) override
	{
		text_ = text;
		//if( hWndEdit )
		//{
		//	::SetWindowTextW(hWndEdit, Utf8ToWstring(text_).c_str());
		//}
		return gmpi::MP_OK;
	}
    
    virtual int32_t MP_STDCALL GetText(IMpUnknown* returnString) override
    {
        gmpi::IString* returnValue = 0;
        
        if (gmpi::MP_OK != returnString->queryInterface(gmpi::MP_IID_RETURNSTRING, reinterpret_cast<void**>( &returnValue)) )
        {
            return gmpi::MP_NOSUPPORT;
        }
        
        returnValue->setData(text_.data(), (int32_t) text_.size());
        return gmpi::MP_OK;
    }
    
//	virtual int32_t MP_STDCALL Show(float x, float y, float w, float h, IMpUnknown* returnString);
    virtual int32_t MP_STDCALL ShowAsync(gmpi_gui::ICompletionCallback* returnCompletionHandler) override;

	void OnEditReturn()
	{
		//		::DestroyWindow(hWndEdit);
	}

	virtual int32_t MP_STDCALL SetAlignment(int32_t alignment) override
	{/ *
		switch( alignment )
		{
		case gmpi_gui::PopupMenu::HorizontalAlignment::A_Left:
			align = TPM_LEFTALIGN;
			break;
		case gmpi_gui::PopupMenu::HorizontalAlignment::A_Center:
			align = TPM_CENTERALIGN;
			break;
		case gmpi_gui::PopupMenu::HorizontalAlignment::A_Right:
		default:
			align = TPM_RIGHTALIGN;
			break;
		}
      * /
		return gmpi::MP_OK;
	}

	GMPI_QUERYINTERFACE1(gmpi_gui::SE_IID_GRAPHICS_PLATFORM_TEXT, gmpi_gui::IMpPlatformText);
	GMPI_REFCOUNT;
};
*/
#endif

} // namespace.
