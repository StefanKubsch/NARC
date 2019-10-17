/*
******************************************
*                                        *
* Game_Config.hpp                        *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
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
		if (const std::string INIFile{ "./DATA/GameConfig/GameConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			const std::map<std::int_fast32_t, std::int_fast32_t> TextureCompare
			{
				{ 64, 6 },
				{ 128, 7 },
				{ 256, 8},
				{ 512, 9},
				{ 1024, 10},
				{ 2048, 11},
				{ 4096, 12},
				{ 8192, 13}
			};

			TextureSize = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "TEXTURES", "TextureSize");

			if (const auto State{ TextureCompare.find(TextureSize) }; State != TextureCompare.end())
			{
				TextureSizeShiftFactor = State->second;
			}
			else
			{
				NARCLog.AddEntry(lwmf::LogLevel::Critical, __FILENAME__, "TextureSize has an incorrect value!");
			}

			EntitySize = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "TEXTURES", "EntitySize");

			if (TextureCompare.find(EntitySize) == TextureCompare.end())
			{
				NARCLog.AddEntry(lwmf::LogLevel::Critical, __FILENAME__, "EntitySize has an incorrect value!");
			}

			FrameLock = lwmf::ReadINIValue<std::uint_fast32_t>(INIFile, "GENERAL", "FrameLock");
		}
	}

	inline void GatherNumberOfLevels()
	{
		NumberOfLevels = 1;

		while (Tools_ErrorHandling::CheckFolderExistence("./DATA/Levels/" + std::to_string(NumberOfLevels), ContinueOnError))
		{
			++NumberOfLevels;
		}

		if (--NumberOfLevels == 0)
		{
			NARCLog.AddEntry(lwmf::LogLevel::Critical, __FILENAME__, "No Leveldata found.");
		}
	}


} // namespace Game_Config