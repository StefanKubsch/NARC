/*
******************************************
*                                        *
* Game_Config.hpp                        *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <map>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"

namespace Game_Config
{


	void Init();
	void GatherNumberOfLevels();

	//
	// Functions
	//

	inline void Init()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init general game config...");

		if (const std::string INIFile{ GameConfigFolder + "GameConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Check used texture sizes...");

			const std::map<std::int_fast32_t, std::int_fast32_t> TextureCompare //-V808
			{
				{ 64, 6 },
				{ 128, 7 },
				{ 256, 8 },
				{ 512, 9 },
				{ 1024, 10 },
				{ 2048, 11 },
				{ 4096, 12 },
				{ 8192, 13 }
			};

			TextureSize = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "TEXTURES", "TextureSize");

			if (const auto State{ TextureCompare.find(TextureSize) }; State != TextureCompare.end())
			{
				TextureSizeShiftFactor = State->second;
			}
			else
			{
				NARCLog.AddEntry(lwmf::LogLevel::Critical, __FILENAME__, __LINE__, "TextureSize has an incorrect value!");
			}

			EntitySize = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "TEXTURES", "EntitySize");

			if (TextureCompare.find(EntitySize) == TextureCompare.end())
			{
				NARCLog.AddEntry(lwmf::LogLevel::Critical, __FILENAME__, __LINE__, "EntitySize has an incorrect value!");
			}

			FrameLock = lwmf::ReadINIValue<std::uint_fast32_t>(INIFile, "GENERAL", "FrameLock");
		}
	}

	inline void GatherNumberOfLevels()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Gathering number of levels...");

		NumberOfLevels = StartLevel;

		while (Tools_ErrorHandling::CheckFolderExistence(LevelFolder + std::to_string(NumberOfLevels), ContinueOnError))
		{
			++NumberOfLevels;
		}

		--NumberOfLevels == StartLevel - 1 ? NARCLog.AddEntry(lwmf::LogLevel::Critical, __FILENAME__, __LINE__, "No Leveldata found.") :
			NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Data of " + std::to_string(NumberOfLevels) + " level(s) was found!");
	}


} // namespace Game_Config