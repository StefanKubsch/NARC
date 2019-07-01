/*
******************************************
*                                        *
* Tools_ErrorHandling.hpp                *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <fstream>
#include <sys/stat.h>

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
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Checking for file existence " + FileName + "...");

		bool Result{ true };

		if (std::ifstream File{ FileName }; File.fail())
		{
			if (ActionFlag == StopOnError)
			{
				NARCLog.AddEntry(lwmf::LogLevel::Error, __FILENAME__, "File not found!");
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
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Checking for folder existence " + FolderName + "...");

		bool Result{ true };

		struct stat Info{};
		stat(FolderName.c_str(), &Info);

		if ((Info.st_mode & S_IFDIR) == 0)
		{
			if (ActionFlag == StopOnError)
			{
				NARCLog.AddEntry(lwmf::LogLevel::Error, __FILENAME__, "Folder not found!");
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
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Checking texture for correct size...");

		bool Result{ true };

		if (Width != Size || Height != Size)
		{
			if (ActionFlag == StopOnError)
			{
				NARCLog.AddEntry(lwmf::LogLevel::Error, __FILENAME__, "TextureSize is " + std::to_string(Width) + " * " + std::to_string(Height) + " pixel!");
			}
			else
			{
				Result = false;
			}
		}

		return Result;
	}


} // namespace Tools_ErrorHandling
