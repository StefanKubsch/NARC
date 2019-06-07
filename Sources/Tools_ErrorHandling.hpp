/*
******************************************
*                                        *
* Tools_ErrorHandling.hpp                *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <cstdint>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include "fmt/format.h"

static constexpr bool ContinueOnError{ true };
static constexpr bool StopOnError{};

namespace Tools_ErrorHandling
{


	bool CheckFileExistence(const std::string& FileName, bool ActionFlag);
	bool CheckFolderExistence(const std::string& FolderName, bool ActionFlag);
	bool CheckTextureSize(std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Size, bool ActionFlag);

	//
	// Functions
	//

	inline bool CheckFileExistence(const std::string& FileName, const bool ActionFlag)
	{
		NARCLog.AddEntry(fmt::format("Checking for file existence {}...", FileName));

		bool Result{ true };

		if (std::ifstream File{ FileName }; File.fail())
		{
			if (ActionFlag == StopOnError)
			{
				NARCLog.LogErrorAndThrowException("ERROR! File not found!");
			}
			else
			{
				Result = false;
			}
		}

		return Result;
	}

	inline bool CheckFolderExistence(const std::string& FolderName, const bool ActionFlag)
	{
		NARCLog.AddEntry(fmt::format("Checking for folder existence {}...", FolderName));

		bool Result{ true };

		struct stat Info{};
		stat(FolderName.c_str(), &Info);

		if ((Info.st_mode & S_IFDIR) == 0)
		{
			if (ActionFlag == StopOnError)
			{
				NARCLog.LogErrorAndThrowException("ERROR! Folder not found!");
			}
			else
			{
				Result = false;
			}
		}

		return Result;
	}

	inline bool CheckTextureSize(const std::int_fast32_t Width, const std::int_fast32_t Height, const std::int_fast32_t Size, const bool ActionFlag)
	{
		NARCLog.AddEntry("Checking texture for correct size...");

		bool Result{ true };

		if (Width != Size || Height != Size)
		{
			if (ActionFlag == StopOnError)
			{
				NARCLog.LogErrorAndThrowException(fmt::format("ERROR! TextureSize is {0}*{1} pixel!", Width, Height));
			}
			else
			{
				Result = false;
			}
		}

		return Result;
	}


} // namespace Tools_ErrorHandling
