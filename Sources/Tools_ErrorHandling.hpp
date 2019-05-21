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

#include "Game_GlobalDefinitions.hpp"
#include "Tools_Console.hpp"

static constexpr bool ShowMessage{ true };
static constexpr bool HideMessage{};
static constexpr bool ContinueOnError{ true };
static constexpr bool StopOnError{};

namespace Tools_ErrorHandling
{


	bool CheckFileExistence(const std::string& FileName, bool MessageFlag, bool ActionFlag);
	bool CheckFolderExistence(const std::string& FolderName, bool MessageFlag, bool ActionFlag);
	bool CheckTextureSize(std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Size, bool MessageFlag, bool ActionFlag);
	void DisplayError(const std::string& Text);
	void DisplayOK();

	//
	// Functions
	//

	inline bool CheckFileExistence(const std::string& FileName, const bool MessageFlag, const bool ActionFlag)
	{
		bool Result{ true };

		if (MessageFlag == ShowMessage)
		{
			Tools_Console::DisplayText(BRIGHT_MAGENTA, fmt::format("Checking for file existence {}...", FileName));
		}

		if (std::ifstream File{ FileName }; File.is_open())
		{
			if (MessageFlag == ShowMessage)
			{
				DisplayOK();
			}
		}
		else
		{
			if (ActionFlag == StopOnError)
			{
				DisplayError("ERROR! File not found!");
			}
			else
			{
				Result = false;
			}
		}

		return Result;
	}

	inline bool CheckFolderExistence(const std::string& FolderName, const bool MessageFlag, const bool ActionFlag)
	{
		bool Result{ true };

		if (MessageFlag == ShowMessage)
		{
			Tools_Console::DisplayText(BRIGHT_MAGENTA, fmt::format("Checking for folder existence {}...", FolderName));
		}

		struct stat Info{};
		stat(FolderName.c_str(), &Info);

		// Folder exists
		if ((Info.st_mode & S_IFDIR) != 0)
		{
			if (MessageFlag == ShowMessage)
			{
				DisplayOK();
			}
		}
		else
		{
			if (ActionFlag == StopOnError)
			{
				DisplayError("ERROR! Folder not found!");
			}
			else
			{
				Result = false;
			}
		}

		return Result;
	}

	inline bool CheckTextureSize(const std::int_fast32_t Width, const std::int_fast32_t Height, const std::int_fast32_t Size, const bool MessageFlag, const bool ActionFlag)
	{
		bool Result{ true };

		if (Debug)
		{
			if (MessageFlag == ShowMessage)
			{
				Tools_Console::DisplayText(BRIGHT_MAGENTA, "Checking texture for correct size...");
			}

			if (Width == Size && Height == Size)
			{
				if (MessageFlag == ShowMessage)
				{
					DisplayOK();
				}
			}
			else
			{
				if (ActionFlag == StopOnError)
				{
					DisplayError(fmt::format("ERROR! TextureSize is {0}*{1} pixel!", Width, Height));
				}
				else
				{
					Result = false;
				}
			}
		}

		return Result;
	}

	inline void DisplayError(const std::string& Text)
	{
		Tools_Console::DisplayText(BRIGHT_RED, fmt::format("\n{}\nPress enter to exit!\n", Text));
		Tools_Console::ClearInputBuffer();
		std::cin.get();
		exit(EXIT_SUCCESS);
	}

	inline void DisplayOK()
	{
		Tools_Console::DisplayText(BRIGHT_GREEN, "Ok!\n");
	}


} // namespace Tools_ErrorHandling
