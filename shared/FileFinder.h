#pragma once
/*
#include "FileFinder.h"

	// useage.
	#include "../shared/FileFinder.h"

	FileFinder it("C:\\temp\\", "wav"); // find all wav files
	for (; !it.done(); ++it)
	{
		if (!(*it).isFolder)
		{
			wstring filenameW = toWstring((*it).filename);
			string filenameUtf8 = toString((*it).filename);
		}
	}

*/

//include headers required for directory traversal
#if defined(_WIN32)
    //disable useless stuff before adding windows.h
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include "dirent.h"
#endif

#include "xplatform.h"

class FileFinder
{
public:
	struct FileFinderItem
	{
		platform_string filename;
		platform_string fullPath;
		bool isFolder;
	};

#if defined(_WIN32)
    FileFinder(const TCHAR* folderPath);
    FileFinder& operator=(const TCHAR* folderPath)
    {
        first(folderPath);
        return *this;
    }
#endif

    FileFinder(const char* folderPath);
	~FileFinder(void);

	FileFinder& operator++()
	{
		next();
		return *this;
	}

	void first( const platform_string& folderPath );
	void next();
	bool done()
	{
		return done_;
	}
	const FileFinderItem& operator*() const
	{
		return current_;
	}
	FileFinderItem& currentItem(void)
	{
		return current_;
	}

private:
#if defined(_WIN32)
	HANDLE directoryHandle;
    WIN32_FIND_DATA fdata;
#else
	DIR* directoryHandle;
    dirent* entry;
#endif

	FileFinderItem current_;
	platform_string searchPath;
    bool done_;
#if defined(_WIN32)
    bool last_;
#else
	platform_string extension;
#endif
};

