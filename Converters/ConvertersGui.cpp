#include "./ConvertersGui.h"

typedef SimpleGuiConverter<int, bool> IntToBool;
typedef SimpleGuiConverter<int, float> IntToFloat;
typedef SimpleGuiConverter<int, std::wstring> IntToText;

typedef SimpleGuiConverter<float, int> FloatToInt;
typedef SimpleGuiConverter<float, bool> FloatToBool;
//typedef SimpleGuiConverter<float, std::wstring> FloatToText;

typedef SimpleGuiConverter<bool, int> BoolToInt;
typedef SimpleGuiConverter<bool, float> BoolToFloat;
typedef SimpleGuiConverter<bool, std::wstring> BoolToText;

typedef SimpleGuiConverter<std::wstring, int> TextToInt;
typedef SimpleGuiConverter<std::wstring, float> TextToFloat;
typedef SimpleGuiConverter<std::wstring, bool> TextToBool;

typedef SimpleGuiConverter<std::wstring, MpBlob> TextToBlob;
typedef SimpleGuiConverter<MpBlob, std::wstring> BlobToText;

REGISTER_GUI_PLUGIN( IntToBool, L"SE IntToBool GUI" );
REGISTER_GUI_PLUGIN( IntToFloat, L"SE IntToFloat GUI" );
REGISTER_GUI_PLUGIN( IntToText, L"SE IntToText GUI" );

REGISTER_GUI_PLUGIN( FloatToInt, L"SE FloatToInt GUI" );
REGISTER_GUI_PLUGIN( FloatToBool, L"SE FloatToBool GUI" );
//REGISTER_GUI_PLUGIN( FloatToText, L"SE FloatToText GUI" );

REGISTER_GUI_PLUGIN( BoolToInt, L"SE BoolToInt GUI" );
REGISTER_GUI_PLUGIN( BoolToFloat, L"SE BoolToFloat GUI" );
REGISTER_GUI_PLUGIN( BoolToText, L"SE BoolToText GUI" );

REGISTER_GUI_PLUGIN( TextToInt, L"SE TextToInt GUI" );
REGISTER_GUI_PLUGIN( TextToFloat, L"SE TextToFloat GUI" );
REGISTER_GUI_PLUGIN( TextToBool, L"SE TextToBool GUI" );

REGISTER_GUI_PLUGIN( TextToBlob, L"SE TextToBlob GUI" );
REGISTER_GUI_PLUGIN( BlobToText, L"SE BlobToText GUI" );

