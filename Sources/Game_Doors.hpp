/*
*****************************************
*                                       *
* Game_Doors.hpp                        *
*                                       *
* (c) 2017, 2018, 2019 Stefan Kubsch    *
*****************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <algorithm>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Game_DataStructures.hpp"
#include "Game_LevelHandling.hpp"
#include "Game_EntityHandling.hpp"

namespace Game_Doors
{


	enum class DoorSounds
	{
		OpenCloseSound
	};

	void InitDoorAssets();
	void InitDoors();
	void UpdateDoors();
	void PlayAudio(const DoorStruct& Door, DoorSounds Sound);
	void CloseAudio();

	//
	// Functions
	//

	inline void InitDoorAssets()
	{
		DoorTypes.clear();
		DoorTypes.shrink_to_fit();
		DoorTypes.resize(1);

		// We start our DoorTypes counting at "1", since in the map definition it has to be greater zero!
		std::int_fast32_t Index{ 1 };

		while (true)
		{
			if (const std::string INIFile{ "./DATA/Assets_Doors/Door_" + std::to_string(Index) + "_Data.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, ContinueOnError))
			{
				DoorTypes.emplace_back();

				lwmf::LoadPNG(DoorTypes[Index].OriginalTexture, lwmf::ReadINIValue<std::string>(INIFile, "TEXTURE", "DoorTexture"));

				DoorTypes[Index].Sounds.emplace_back();
				DoorTypes[Index].Sounds[static_cast<std::int_fast32_t>(DoorSounds::OpenCloseSound)].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "OpenCloseSound"));

				++Index;
			}
			else
			{
				NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "No more doortype data found.");
				break;
			}
		}
	}

	inline void InitDoors()
	{
		Doors.clear();
		Doors.shrink_to_fit();

		for (std::int_fast32_t Index{}, MapPosX{}; MapPosX < Game_LevelHandling::LevelMapWidth; ++MapPosX)
		{
			for (std::int_fast32_t MapPosY{}; MapPosY < Game_LevelHandling::LevelMapHeight; ++MapPosY)
			{
				const std::int_fast32_t FoundDoorType{ Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Door)][MapPosX][MapPosY] };

				if (FoundDoorType > 0)
				{
					Doors.emplace_back();

					Doors[Index].DoorType = FoundDoorType;
					Doors[Index].AnimTexture = DoorTypes[FoundDoorType].OriginalTexture;
					Doors[Index].Number = Index;
					Doors[Index].Pos = { static_cast<float>(MapPosX), static_cast<float>(MapPosY) };
					Doors[Index].OpenPercent = 1;

					++Index;
				}
			}
		}
	}

	inline void UpdateDoors()
	{
		for (auto&& Door : Doors)
		{
			const lwmf::FloatPointStruct Distance{ Player.Pos.X - (Door.Pos.X + 0.5F), Player.Pos.Y - (Door.Pos.Y + 0.5F) };
			const float dtd{ std::abs(Distance.X * Distance.X + Distance.Y + Distance.Y) };

			if (dtd < 2)
			{
				if (Door.OpenPercent < 100.0F)
				{
					Door.OpenVelocity = 2;
				}
			}
			else
			{
				if (Door.OpenPercent > 0.0F)
				{
					Door.OpenVelocity = -2;
				}
			}

			Door.OpenPercent = std::clamp(Door.OpenPercent + Door.OpenVelocity, 0.0F, 100.0F);

			for (std::int_fast32_t y{}; y < TextureSize; ++y)
			{
				const std::int_fast32_t TempY{ y * TextureSize };

				for (std::int_fast32_t SourceTextureX{}, x{ static_cast<std::int_fast32_t>(Door.OpenPercent) }; x < TextureSize; ++x, ++SourceTextureX)
				{
					Door.AnimTexture.Pixels[TempY + x] = DoorTypes[Door.DoorType].OriginalTexture.Pixels[TempY + SourceTextureX];
				}
			}
		}
	}

	inline void PlayAudio(const DoorStruct& Door, const DoorSounds Sound)
	{
		DoorTypes[Door.DoorType].Sounds[static_cast<std::int_fast32_t>(Sound)].Play(lwmf::MP3::PlayModes::FROMSTART);
	}

	inline void CloseAudio()
	{
		for (auto&& DoorType : DoorTypes)
		{
			for (auto&& Sound : DoorType.Sounds)
			{
				Sound.Close();
			}
		}
	}


} // namespace Game_Doors
