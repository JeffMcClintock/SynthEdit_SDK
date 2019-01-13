#include "RiffFile2.h"
#include <assert.h>

RiffFile2::RiffFile2() :
	fileHandle(nullptr)
	, level(0)
	, rec_chunks()
{
}

bool RiffFile2::Open( gmpi::IProtectedFile* file, uint32_t& riff_type )
{
	filePos = 0;
	riff_type = 0;
	
	fileHandle = file;

	if (fileHandle == nullptr)
		return true;

	return Open2(riff_type);
}

bool RiffFile2::Open2( uint32_t& riff_type)
{
	level = 0;

	// Get first chunk (describes file)
//	int res = mmioDescend(fileHandle, &chnk, NULL,NULL);

	MMCKINFO_SE& chunk = cnk[level];

	fileHandle->read((char*)&(chunk.ckid), sizeof(chunk.ckid));
	filePos += sizeof(chunk.ckid);
	char* chunkName = (char*) &(chunk.ckid);

	if (chunkName[0] != 'R' || chunkName[1] != 'I' || chunkName[2] != 'F' || chunkName[3] != 'F') // RIFF.
	{
//		throw("Input stream doesn't comply with the RIFF specification");
		return true;
	}
	fileHandle->read((char*)&(chunk.cksize), sizeof(chunk.cksize));
	filePos += sizeof(chunk.cksize);

	chunk.fccType = 0;

    const uint32_t RIFF = MAKEFOURCC('R', 'I', 'F', 'F');
    
	// Check this is a RIFF file
//	if(chunk.ckid != RIFF) //MAKEFOURCC('R', 'I', 'F', 'F') )
//		return true;

	// Read po (int) testfirst chunk type
	fileHandle->read((char*)&(chunk.fccType), sizeof(chunk.fccType));
	filePos += sizeof(chunk.fccType);
	riff_type = chunk.fccType;

//	char *j1 = (char*) &(chnk.fccType);
//	char *j2 = (char*) &(chnk.ckid);
//	_RPT4(_CRT_WARN, "'%c%c%c%c' \n",j2[0],j2[1],j2[2],j2[3]);
//	_RPT4(_CRT_WARN, "'%c%c%c%c' \n",j1[0],j1[1],j1[2],j1[3]);

	return false;
}

void RiffFile2::AddChunk( riff_match riff_desc )
{
	rec_chunks.push_back(riff_desc);
}

bool RiffFile2::ReadFile()
{
	int res = 0;
	MMCKINFO_SE chunk;

	int32_t totalFileSize;
	fileHandle->getSize(totalFileSize);

	// cnk[level] keeps track of depth of current chunk (usually no more than 1)
	while (filePos < totalFileSize - (sizeof(chunk.ckid) + sizeof(chunk.cksize)))
	{
		// Read a header.
		fileHandle->read((char*)&(chunk.ckid), sizeof(chunk.ckid));
		filePos += sizeof(chunk.ckid);
		fileHandle->read((char*)&(chunk.cksize), sizeof(chunk.cksize));
		filePos += sizeof(chunk.cksize);

		if (chunk.ckid == MAKEFOURCC('L', 'I', 'S', 'T'))
		{
			// NOTE: Ascending from chunk not implemented.
			++level;
			fileHandle->read((char*)&(chunk.fccType), sizeof(chunk.fccType));
			filePos += sizeof(chunk.fccType);
			cnk[level] = chunk;
		}
		else
		{
			auto datasize = chunk.cksize;
			char* chunkName = (char*) &(chunk.ckid);
			char* chunkType = (char*) &(chunk.fccType);
//			_RPT4(_CRT_WARN, "'%c%c%c%c' ", chunkName[0], chunkName[1], chunkName[2], chunkName[3]);
//			_RPT4(_CRT_WARN, "'%c%c%c%c' ", chunkType[0], chunkType[1], chunkType[2], chunkType[3]);
//			_RPT1(_CRT_WARN, "%d bytes \n", chunk.cksize);

			cnk[level] = chunk;

			{
				//char* chunkName = (char*) &(chunk.ckid);
				//_RPT4(_CRT_WARN, "'%c%c%c%c' ", chunkName[0], chunkName[1], chunkName[2], chunkName[3]);
				//_RPT1(_CRT_WARN, "%d bytes \n", chunk.cksize);

				bool dataRead = false;

				// Handle specific chunk types.
				for (int j = (int)rec_chunks.size() - 1; j >= 0; j--)
				{
					if (rec_chunks[j].fccType == cnk[level].fccType && rec_chunks[j].ckid == chunk.ckid)
					{
						if (rec_chunks[j].count_addr)	// caller wants chunk size (in specific units)
						{
							*(rec_chunks[j].count_addr) = chunk.cksize / rec_chunks[j].item_size;
							assert(chunk.cksize % rec_chunks[j].item_size == 0);	// CHUNK SIZE ERROR!
						}

						if (rec_chunks[j].addr)	// caller wants chunk read into memory
						{
							assert(*(rec_chunks[j].addr) == 0); // overwriting existing pointer!!
							*(rec_chunks[j].addr) = new char[chunk.cksize];

//								memcpy(*(rec_chunks[j].addr), data.data(), data.size());
							fileHandle->read(*(rec_chunks[j].addr), chunk.cksize);
							filePos += chunk.cksize;
							//						res = mmioRead(file_handle, (HPSTR) *(rec_chunks[j].addr), chnk.cksize);
							//						assert(res == chnk.cksize);
													/*
													// debug output of result
													int print_words = chnk.cksize / 4;
													print_words = min(print_words,20);
													DWORD *ptr = (DWORD*) *(rec_chunks[j].addr);
													for(int i = 0 ; i < print_words ; i++ )
													{
													_RPT1(_CRT_WARN, "%08x ", *ptr++ );
													}
													_RPT0(_CRT_WARN, "\n" );
													*/

							dataRead = true;
						}
						break;
					}
				}

				if (!dataRead)
				{
					std::string data;
					data.resize(datasize);
					fileHandle->read((char*)data.data(), datasize);
					filePos += datasize;
				}
			}
		}
	}
	
	return 0;
}

