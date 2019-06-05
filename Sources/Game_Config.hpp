/*
******************************************
*                                        *
* Game_Config.hpp                        *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <cstdint>
#include <string>
#include "fmt/format.h"

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"

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
			TextureSize = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "TEXTURES", "TextureSize");
			EntitySize = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "TEXTURES", "EntitySize");
			FrameLock = Tools_INIFile::ReadValue<std::uint_fast32_t>(INIFile, "GENERAL", "FrameLock");

			// Factor for bitwise texture operations
			TextureSizeBitwiseAnd = TextureSize - 1;

			// Set factor for bitshifting from TextureSize
			switch (TextureSize)
			{
				case 64:
				{
					TextureSizeShiftFactor = 6;
					break;
				}
				case 128:
				{
					TextureSizeShiftFactor = 7;
					break;
				}
				case 256:
				{
					TextureSizeShiftFactor = 8;
					break;
				}
				case 512:
				{
					TextureSizeShiftFactor = 9;
					break;
				}
				case 1024:
				{
					TextureSizeShiftFactor = 10;
					break;
				}
				case 2048:
				{
					TextureSizeShiftFactor = 11;
					break;
				}

				default:{}
			}
		}
	}

	inline void GatherNumberOfLevels()
	{
		NumberOfLevels = 0;

		while (true)
		{
			if (Tools_ErrorHandling::CheckFolderExistence(fmt::format("./DATA/Level_{}", NumberOfLevels), ContinueOnError))
			{
				++NumberOfLevels;
			}
			else
			{
				break;
			}
		}

		// Reduce NumberOfLevels by 1 since we start counting our levels by 0 (Zero) !
		if (--NumberOfLevels == -1)
		{
			lwmf::LogErrorAndThrowException("No Leveldata found.");
		}
	}


} // namespace Game_Config