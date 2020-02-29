/*
******************************************
*                                        *
* Tools_ErrorHandling.hpp                *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>

constexpr bool ContinueOnError{ true };
constexpr bool StopOnError{};

namespace Tools_ErrorHandling
{


	bool CheckFileExistence(const std::string& FileName, bool ActionFlag);
	bool CheckFolderExistence(const std::string& FolderName, bool ActionFlag);
	bool CheckTextureSize(std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Size, bool ActionFlag);
	template<typename T>void CheckAndClampRange(T& Value, T Min, T Max, const std::string& File, const std::string& VariableName);

	//
	// Functions
	//

	inline bool CheckFileExistence(const std::string& FileName, const bool ActionFlag)
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Checking for file existence " + FileName + "...");

		bool Result{ true };

		std::ifstream File(FileName, std::ios::in);

		if (File.fail())
		{
			std::array<char, 100> ErrorMessage{};
			strerror_s(ErrorMessage.data(), 100, errno);

			if (ActionFlag == StopOnError)
			{
				NARCLog.AddEntry(lwmf::LogLevel::Error, __FILENAME__, "Error loading " + FileName + ": " + std::string(ErrorMessage.data()));
			}
			else
			{
				NARCLog.AddEntry(lwmf::LogLevel::Warn, __FILENAME__, "Error loading " + FileName + ": " + std::string(ErrorMessage.data()));
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
				NARCLog.AddEntry(lwmf::LogLevel::Warn, __FILENAME__, "Folder not found!");
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
				NARCLog.AddEntry(lwmf::LogLevel::Warn, __FILENAME__, "TextureSize is " + std::to_string(Width) + " * " + std::to_string(Height) + " pixel!");
				Result = false;
			}
		}

		return Result;
	}

	template<typename T>void CheckAndClampRange(T& Value, const T Min, const T Max, const std::string& File, const std::string& VariableName)
	{
		if (Value < Min || Value > Max)
		{
			Value = std::clamp(Value, Min, Max);
			NARCLog.AddEntry(lwmf::LogLevel::Warn, File.c_str(), VariableName + " needs to be between " + std::to_string(Min) + " and " + std::to_string(Max) + ". Value was clamped to given range.");
		}
	}


} // namespace Tools_ErrorHandling
