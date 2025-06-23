#ifndef RESOURCE_LOADER_SATURN_INCLUDE_H
#define RESOURCE_LOADER_SATURN_INCLUDE_H

#include <srl.hpp>

extern "C"
{
#include "base_types.h"
#include "resource_types.h"
#include "base_defines.h"
#include "alloc.h"
#include "resource_loader_buffer.h"
#include "checksum_utils.h"
}

#define RESULT_OK						0
#define RESULT_FILENOTFOUND				1
#define RESULT_CHECKSUMFAILED			2
#define RESULT_RESOURCELOADER_FAILED	3
#define RESULT_UNKNOWNFAILURE			0xff

class DownlandResourceLoader
{
public:
	static int LoadResources(const char* romPath, Resources* resources)
	{
		u8* fileBuffer = NULL;
		u32 fileSize = 0;
		
		if (!loadFile(romPath, &fileBuffer, &fileSize))
			return RESULT_FILENOTFOUND;

		if (fileBuffer == NULL)
			return RESULT_FILENOTFOUND;

		if (!checksumCheckBigEndian(fileBuffer, fileSize))
			return RESULT_CHECKSUMFAILED;

		if (!ResourceLoaderBuffer_Init(fileBuffer, fileSize, resources))
			return RESULT_RESOURCELOADER_FAILED;

		return RESULT_OK;
	}

private:
	static bool loadFile(const char* romPath, u8** fileBuffer, u32* fileSize)
	{
		SRL::Cd::File file = SRL::Cd::File(romPath);

		if (!file.Exists())
		{
			return false;
		}

		*fileSize = file.Size.Bytes;
		file.Open();

		*fileBuffer = (u8*)dl_alloc(*fileSize);
		file.Read(*fileSize, *fileBuffer);

		file.Close();

		return true;
	}
};

#endif 