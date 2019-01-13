#pragma once

#include "xp_dynamic_linking.h"


// Provide a cross-platform loading of dlls.

#if defined(_WIN32)
#include "windows.h"
#else
#include <dlfcn.h>
#include "unicode_conversion.h"
using namespace JmUnicodeConversions;
#endif

namespace gmpi_dynamic_linking
{

#if defined(_WIN32)
	typedef HINSTANCE MP_DllHandle;
#else
//	typedef CFBundleRef MP_DllHandle;
	typedef void* MP_DllHandle;
#endif

	int32_t MP_DllLoad(DLL_HANDLE* dll_handle, const wchar_t* dll_filename)
	{
#if defined( _WIN32)
	#if (defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_APP )
			*dll_handle = 0;
			// !!TODO!! LoadPackagedLibrary( dlls packaged with app only).
	#else
			*dll_handle = (DLL_HANDLE) LoadLibraryW(dll_filename);
	#endif
#else
		*dll_handle = (DLL_HANDLE) dlopen(WStringToUtf8(dll_filename).c_str(), 0);
#endif
		return *dll_handle == 0;
	}

	int32_t MP_DllUnload(DLL_HANDLE dll_handle)
	{
		int32_t r = 0;
		if (dll_handle)
		{
#if defined( _WIN32)
			r = FreeLibrary((HMODULE)dll_handle);
#else
			r = dlclose((MP_DllHandle)dll_handle);
#endif
		}
		return r == 0;
	}

	int32_t MP_DllSymbol(DLL_HANDLE dll_handle, const char* symbol_name, void** returnFunction)
	{
#if defined( _WIN32)
		*returnFunction = (void*) GetProcAddress((HMODULE)dll_handle, symbol_name);
#else
		*returnFunction = dlsym((MP_DllHandle) dll_handle, symbol_name);
#endif
		return *returnFunction == 0;
	}

    // Provide a static function to allow GetModuleHandleExA() to find dll name.
    void localFuncWithUNlikelyName3456()
    {
    }

#if defined(_WIN32)
#if (!defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP ) // Desktop Windows only.

	int32_t MP_GetDllHandle(DLL_HANDLE* returnDllHandle)
	{
		HMODULE hmodule = 0;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&localFuncWithUNlikelyName3456, &hmodule);
		*returnDllHandle = ( DLL_HANDLE) hmodule;
		return 1;
	}

	std::wstring MP_GetDllFilename()
	{
		DLL_HANDLE hmodule = 0;
		MP_GetDllHandle(&hmodule);

		wchar_t full_path[MAX_PATH] = L"";
		GetModuleFileNameW((HMODULE)hmodule, full_path, sizeof(full_path));
		return std::wstring(full_path);
	}
#else
	int32_t MP_GetDllHandle(DLL_HANDLE* returnDllHandle)
	{
		*returnDllHandle = 0; // TODO
		return 1;
	}

	std::wstring MP_GetDllFilename()
	{
		return std::wstring(); // TODO
	}

#endif
    
#else
    // Mac
    std::wstring MP_GetDllFilename()
    {
        Dl_info info;
        int rv = dladdr((void *)&localFuncWithUNlikelyName3456, &info);
        assert(rv != 0);

        return Utf8ToWstring(info.dli_fname);
    }
#endif
}
