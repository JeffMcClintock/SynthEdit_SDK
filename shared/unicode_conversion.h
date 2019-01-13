#ifndef UNICODE_CONVERSION_H_INCLUDED
#define UNICODE_CONVERSION_H_INCLUDED

/*
#include "../shared/unicode_conversion.h"

using namespace JmUnicodeConversions;
*/

#include <string>
#include <assert.h>
#include <stdlib.h>	 // wcstombs() on Linux.
#include "xplatform.h"

#if defined(_WIN32)
#include "windows.h"
#endif

namespace JmUnicodeConversions
{

inline std::string WStringToUtf8(const std::wstring& p_cstring )
{
#if defined(_WIN32)
	size_t bytes_required = 1 + WideCharToMultiByte( CP_UTF8, 0, p_cstring.c_str(), -1, 0, 0, NULL, NULL);
#else
	size_t bytes_required = 1 + p_cstring.size();
#endif

	char* temp = new char[bytes_required];
    
#if defined(_WIN32)
	WideCharToMultiByte( CP_UTF8, 0, p_cstring.c_str(), -1, temp, (int) bytes_required, NULL, NULL);
#else
	wcstombs(temp, p_cstring.c_str(), bytes_required );
#endif

	std::string res(temp);
	delete [] temp;
	return res;
}

inline std::wstring Utf8ToWstring( const char* p_string )
{
    if( p_string == 0)
    {
        return std::wstring(L"");
    }
#if defined(_WIN32)
	size_t length = 1 + MultiByteToWideChar( CP_UTF8, 0, p_string, -1, (LPWSTR)0, 0 );
#else
	size_t length = 1 + mbstowcs(0, p_string, 0 );
#endif

	wchar_t* wide = new wchar_t[length];
	wide[0] = 0; // Handle null input pointers.

#if defined(_WIN32)
	MultiByteToWideChar( CP_UTF8, 0, p_string, -1, (LPWSTR)wide, (int) length );
#else
	mbstowcs(wide, p_string, length );
#endif

	std::wstring temp(wide);
	delete [] wide;
	return temp;
}

inline std::wstring Utf8ToWstring( const std::string& p_string )
{
	return Utf8ToWstring( p_string.c_str() );
}

#ifdef _WIN32
    
    inline std::wstring ToUtf16( const std::wstring& s )
    {
        assert( sizeof(wchar_t) == 2 );
        return s;
    }
    
#else
/*
#ifdef __INTEL__
    typedef char16_t TChar;
#else
    typedef unsigned short TChar;
#endif
 */
#if __cplusplus <= 199711L // condition for new mac
    typedef unsigned short TChar;
#else
    typedef char16_t TChar;
#endif
    
    typedef std::basic_string<TChar, std::char_traits<TChar>, std::allocator<TChar> > utf16_string;
    
    inline utf16_string ToUtf16( const std::wstring& s )
    {
        // On Mac. Wide-string is 32-bit.Lame conversion.
        utf16_string r;
        r.resize( s.size() );
        
        TChar* dest = (TChar*) r.data();
        for( size_t i = 0 ; i < s.size() ; ++i )
        {
            *dest++ = (TChar) s[i];
        }
        return r;
    }
    
#endif
    
#ifdef UNICODE
	inline std::wstring toWstring( const platform_string& s )
	{
		return s;
	}
	inline std::string toString( const platform_string& s )
	{
		return WStringToUtf8(s);
	}
	inline platform_string toPlatformString( const std::wstring& s )
	{
		return s;
	}
	inline platform_string toPlatformString( const std::string& s )
	{
		return Utf8ToWstring(s);
	}
#else
	inline std::string toString( const platform_string& s )
	{
		return s;
	}
	inline std::wstring toWstring( const platform_string& s )
	{
		return Utf8ToWstring(s);
	}
    inline std::wstring toWstring( const _TCHAR* s )
    {
        return toWstring( platform_string(s) );
    }
	inline platform_string toPlatformString( const std::string& s )
	{
		return s;
	}
	inline platform_string toPlatformString( const std::wstring& s )
	{
		return WStringToUtf8(s);
	}
#endif

}
#endif
