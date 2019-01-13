#include "./SampleLoader2.h"
#include "./SampleManager.h"

REGISTER_PLUGIN ( SampleLoader2, L"SE Sample Loader2" );

SampleLoader2::SampleLoader2( IMpUnknown* host ) : MpBase( host )
,sampleHandle(-1)
{
	// Register pins.
	initializePin( 0, pinFilename );
	initializePin( 1, pinBank );
	initializePin( 2, pinPatch );
	initializePin( 3, pinSampleId );
}

SampleLoader2::~SampleLoader2()
{
	SampleManager::Instance()->Release( sampleHandle );
}

void SampleLoader2::onSetPins(void)
{
	if( pinFilename.isUpdated() || pinBank.isUpdated() || pinPatch.isUpdated() )
	{
		int oldSamplehandle = sampleHandle;

		// resolve full filename path acording to host's prefered folder.
		const int maxCharacters = 500;
		wchar_t fullFilename[maxCharacters];
		getHost()->resolveFilename( pinFilename.getValue().c_str(), maxCharacters, fullFilename );

		// open imbedded file.
		gmpi::IProtectedFile* file = 0;
		int r = getHost()->openProtectedFile( pinFilename.getValue().c_str(), &file );

		if( r == gmpi::MP_OK )
		{
/* testing stream based appraoch, for better compatibility with standard c++.
		    protected_buffer< char > buf (file);
			std::basic_istream< char > myfile( &buf );

int size;
file->getSize(size);

char* test  = new char[size];
myfile.get(test,size);
if( size < 1000 )
{
		_RPT1(_CRT_WARN, "%s\n", test );

}
delete [] test;
file->close();
return;
*/
			// Load soundfont.
			sampleHandle = SampleManager::Instance()->Load( file, fullFilename, pinBank, pinPatch  );
			file->close();
		}
		else
		{
			 sampleHandle = -1;
		}

		pinSampleId = sampleHandle;

		// Release old sample last. So it stays in memory in case 'new' sample
		// comes from same soundfont. (else soundfont is unloaded/reloaded unnesesarily).
		SampleManager::Instance()->Release( oldSamplehandle );
	}
}

