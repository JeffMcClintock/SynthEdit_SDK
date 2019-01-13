#include "./my_type_convert.h"

#if _MSC_VER >= 1600 // Not Avail in VS2005.
//#include <locale>
#include <codecvt>
//#include <cvt//wstring>
#endif

using namespace std;

// specialisations for types that need more complex converting.
template<>
bool myTypeConvert( int value )
{
	return value != 0;
};

template<>
bool myTypeConvert( float value )
{
	return value > 0.0f;
};

template<>
bool myTypeConvert( wstring value )
{
	return wcscmp( value.c_str(), L"1") == 0;
};

template<>
int myTypeConvert( wstring value )
{
	return wcstol( value.c_str(), 0 , 10 );
};

template<>
float myTypeConvert( wstring value )
{
	return (float) wcstod( value.c_str(), 0 );
};

template<>
wstring myTypeConvert( bool value )
{
	if( value )
		return L"1";
	return L"0";
};

template<>
wstring myTypeConvert( int value )
{
	const int maxSize = 50;
	wchar_t stringval[maxSize];
#if defined(_MSC_VER)
	swprintf_s( stringval, maxSize, L"%d", value );
#else
	swprintf( stringval, maxSize, L"%d", value );
#endif
	return stringval;
};

template<>
wstring myTypeConvert( float value )
{
	const int maxSize = 50;
	wchar_t stringval[maxSize];
#if defined(_MSC_VER)
	swprintf_s( stringval, maxSize, L"%f", value );
#else
	swprintf( stringval, maxSize, L"%f", value );
#endif

	// Eliminate Trailing Zeros
	wchar_t *p = 0;
	for (p = stringval; *p; ++p) {
		if (L'.' == *p) {
			while (*++p);
			while (L'0' == *--p) *p = L'\0';
			if (*p == L'.') *p = L'\0';
			break;
		}
	}

	return stringval;
};

template<>
wstring myTypeConvert( MpBlob value )
{
	int size = value.getSize();

	wchar_t* outputString = new wchar_t[size * 3 + 1];
	outputString[0] = 0; // null terminate incase blob empty.

	unsigned char* blobData = (unsigned char*) value.getData();
	for(int i = 0 ; i < size ; ++i )
	{
		// Use safe printf if available.
		#if defined(_MSC_VER)
			swprintf_s( outputString + i * 3, 4, L"%02X ", blobData[i] );
		#else
			swprintf( outputString + i * 3, 4, L"%02X ", blobData[i] );
		#endif
	}
	
	wstring outputValue;

	outputValue = outputString;

	delete [] outputString;
	return outputValue;
};

template<>
MpBlob myTypeConvert( wstring value )
{
	auto textlength = value.size();

	// estimate max blob size as helf string length.
	unsigned char* blobData = new unsigned char[1 + textlength / 2]; // round up odd numbers.

	int blobSize = 0;

	bool hiByte = true;

	unsigned char binaryValue = 0;

	for(int i = 0 ; i < textlength ; ++i )
	{
		wchar_t c = value[i];
		int nybbleVal = 0;

		if( c != L' ' )
		{
			if( c >= L'0' && c <= L'9' )
			{
				nybbleVal = c - L'0';
			}
			else
			{
				if( c >= L'A' && c <= L'F' )
				{
					nybbleVal = 10 + c - L'A';
				}
				else
				{
					if( c >= L'a' && c <= L'f' )
					{
						nybbleVal = 10 + c - L'a';
					}
				}
			}

			if( hiByte )
			{
				binaryValue = nybbleVal;
			}
			else
			{
				binaryValue = (binaryValue << 4 ) + nybbleVal;

				blobData[blobSize++] = binaryValue;
				binaryValue = 0;
			}

			hiByte = !hiByte;
		}
	}

	// any odd char on end?, include it.
	if( !hiByte )
	{
		blobData[blobSize++] = binaryValue;
	}
	
	MpBlob temp( blobSize, blobData );

	delete [] blobData;

	return temp;
}

string UnicodeToAscii(wstring s) // Actually to UTF8.
{
	if( s.empty() )
	{
		return string("");
	}
/*
	int string_length = s.size();
	char* temp = new char[string_length];
	WideCharToMultiByte(CP_ACP, 0, s.c_str(), -1, temp, string_length, NULL, NULL);

	std::string returnValue( temp, string_length );
	delete [] temp;

	return returnValue;
*/
#if _MSC_VER >= 1600 // Not Avail in VS2005.
	std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
	return convert.to_bytes(s);
    
#else
    
    std::string out;
    unsigned int codepoint = 0;
    for (const wchar_t* in = s.c_str(); *in != 0;  ++in)
    {
        if (*in >= 0xd800 && *in <= 0xdbff)
            codepoint = ((*in - 0xd800) << 10) + 0x10000;
        else
        {
            if (*in >= 0xdc00 && *in <= 0xdfff)
                codepoint |= *in - 0xdc00;
            else
                codepoint = *in;
            
            if (codepoint <= 0x7f)
                out.append(1, static_cast<char>(codepoint));
            else if (codepoint <= 0x7ff)
            {
                out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
                out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
            }
            else if (codepoint <= 0xffff)
            {
                out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
                out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
                out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
            }
            else
            {
                out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
                out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
                out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
                out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
            }
            codepoint = 0;
        }
    }
    return out;
#endif
}