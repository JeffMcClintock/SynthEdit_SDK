#pragma once
/*
#include "../shared/unicode_conversion.h"

using namespace JmUnicodeConversions;
*/

#include <string>
#include <assert.h>
#include <stdlib.h>	 // wcstombs() on Linux.
#if defined(_WIN32)
#include "windows.h"
#endif

namespace JmUnicodeConversions
{

inline std::string WStringToUtf8(const std::wstring& p_cstring )
{
#if defined(_WIN32)
    std::string res;
    const size_t size = WideCharToMultiByte(
		CP_UTF8,
		0,
		p_cstring.data(),
		static_cast<int>(p_cstring.size()),
		0,
		0,
		NULL,
		NULL
	);
    
	res.resize(size);

	WideCharToMultiByte(
		CP_UTF8,
		0,
		p_cstring.data(),
		static_cast<int>(p_cstring.size()),
		const_cast<LPSTR>(res.data()),
		static_cast<int>(size),
		NULL,
		NULL
	);
	return res;
#else
	const auto size = wcstombs(0, p_cstring.c_str(), 0);
	std::string res;
	res.resize(size);
	wcstombs((char*)res.data(), p_cstring.c_str(), size);
	return res;
#endif
}

inline std::wstring Utf8ToWstring(const char* pstr, size_t psize)
{
#if defined(_WIN32)
	const size_t size = MultiByteToWideChar(
		CP_UTF8,
		0,
		pstr,
		static_cast<int>(psize),
		0,
		0
	);

	std::wstring res;
	res.resize(size);

	MultiByteToWideChar(
		CP_UTF8,
		0,
		pstr,
		static_cast<int>(psize),
		const_cast<LPWSTR>(res.data()),
		static_cast<int>(size)
	);
#else
	const auto size = mbstowcs(0, pstr, 0);
	std::wstring res;
	res.resize(size);
	mbstowcs(const_cast<wchar_t*>(res.data()), pstr, size);
#endif
	return res;
}

inline std::wstring Utf8ToWstring(const std::string& p_string)
{
	return Utf8ToWstring(p_string.data(), p_string.size());
}

inline std::wstring Utf8ToWstring(const char* p_string)
{
	return Utf8ToWstring(p_string, strlen(p_string));
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
}
