#include ".\UserSettingTextGui.h"
#include "shlobj.h"
#include "tinyxml/tinyxml.h"
#include "string"
#include <direct.h>

REGISTER_GUI_PLUGIN( UserSettingTextGui, L"SE UserSettingText" );

std::string WStringToUtf8(const std::wstring& p_cstring )
{
	int bytes_required = 1 + WideCharToMultiByte( CP_UTF8, 0, p_cstring.c_str(), -1, 0, 0, NULL, NULL);
	char* temp = new char[bytes_required];
	WideCharToMultiByte( CP_UTF8, 0, p_cstring.c_str(), -1, temp, bytes_required, NULL, NULL);
	std::string res(temp);
	delete [] temp;
	return res;
}

std::wstring Utf8ToWstring( char const* p_string )
{
#if defined(WIN32)
	int length = 1 + MultiByteToWideChar( CP_UTF8, 0, p_string, -1, (LPWSTR)0, 0 );
	wchar_t* wide = new wchar_t[length];
	wide[0] = 0; // Handle null input pointers.
	MultiByteToWideChar( CP_UTF8, 0, p_string, -1, (LPWSTR)wide, length );
	std::wstring temp(wide);
	delete [] wide;
	return temp;
#else
	// TODO.
#endif
}


UserSettingTextGui::UserSettingTextGui( IMpUnknown* host ) : MpGuiBase(host)
{
	// initialise pins.
	pinProduct.initialize( this, 0);
	pinKey.initialize( this, 1);
	pinDefault.initialize( this, 2);
	pinValue.initialize( this, 3, static_cast<MpGuiBaseMemberPtr>(&UserSettingTextGui::onSetValue) );
}

// Determine settings file: C:\Users\Jeff\AppData\Local\Plugin\Preferences.xml
std::string UserSettingTextGui::getSettingFilePath()
{
	wchar_t mySettingsPath[MAX_PATH];
	SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, mySettingsPath );
	std::wstring meSettingsFile( mySettingsPath );
	meSettingsFile += L"/";
	meSettingsFile += pinProduct.getValue();

	// Create folder if not already.
	_wmkdir( meSettingsFile.c_str() );

	meSettingsFile += L"/Preferences.xml";

	return  WStringToUtf8(meSettingsFile);
}

int32_t UserSettingTextGui::initialize()
{
	TiXmlDocument doc( getSettingFilePath() );

	if( doc.LoadFile() && !doc.Error() )
	{
		TiXmlNode* settingsXml = doc.FirstChild( "Preferences" );

		if( settingsXml )
		{
			TiXmlElement* keyXml = settingsXml->FirstChildElement( WStringToUtf8(pinKey).c_str() );

			if( keyXml )
			{
				pinValue = Utf8ToWstring( keyXml->GetText() );
				return gmpi::MP_OK;
			}
		}
	}

	pinValue = pinDefault;
	return gmpi::MP_OK;
}

void UserSettingTextGui::onSetValue()
{
	// pinValue changed. Store it in settings file.
	TiXmlDocument doc;
	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild( decl );
	TiXmlElement* element_preferences = new TiXmlElement( "Preferences" );
	doc.LinkEndChild( element_preferences );
	TiXmlElement* element_key = new TiXmlElement( WStringToUtf8(pinKey) );
	element_preferences->LinkEndChild( element_key );
	element_key->LinkEndChild( new TiXmlText( WStringToUtf8(pinValue) ) );

	doc.SaveFile( getSettingFilePath() );
}
