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
	void TriggerDoor();
	void ModifyDoorTexture(DoorStruct& Door);
	void OpenCloseDoors();
	void PlayAudio(const DoorStruct& Door, DoorSounds Sound);
	void CloseAudio();

	//
	// Variables and constants
	//

	static constexpr float MaximumOpenPercentMin{ 1.0F };
	static constexpr float MaximumOpenPercentMax{ 100.0F };
	static constexpr float MinimumOpenPercentMin{ 0.0F };
	static constexpr float MinimumOpenPercentMax{ 99.0F };

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

				DoorTypes[Index].OpenCloseSpeed = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "OpenCloseSpeed");
				DoorTypes[Index].StayOpenTime = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "StayOpenTime") * static_cast<std::int_fast32_t>(FrameLock);
				DoorTypes[Index].MaximumOpenPercent = lwmf::ReadINIValue<float>(INIFile, "GENERAL", "MaximumOpenPercent");
				DoorTypes[Index].MinimumOpenPercent = lwmf::ReadINIValue<float>(INIFile, "GENERAL", "MinimumOpenPercent");

				Tools_ErrorHandling::CheckAndClampRange(DoorTypes[Index].MaximumOpenPercent, MaximumOpenPercentMin, MaximumOpenPercentMax, __FILENAME__, "MaximumOpenPercent");
				Tools_ErrorHandling::CheckAndClampRange(DoorTypes[Index].MinimumOpenPercent, MinimumOpenPercentMin, MinimumOpenPercentMax, __FILENAME__, "MinimumOpenPercent");

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

					Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][MapPosX][MapPosY] = INT_MAX;

					++Index;
				}
			}
		}
	}

	inline void TriggerDoor()
	{
		for (auto&& Door : Doors)
		{
			if (!Door.IsOpenTriggered && !Door.IsOpen && (std::abs(Door.Pos.X - Player.FuturePos.X) < FLT_EPSILON && std::abs(Door.Pos.Y - Player.FuturePos.Y) < FLT_EPSILON))
			{
				Door.IsOpenTriggered = true;
				Door.CurrentOpenPercent = DoorTypes[Door.DoorType].MinimumOpenPercent;
				PlayAudio(Door, DoorSounds::OpenCloseSound);
				break;
			}
		}
	}

	inline void ModifyDoorTexture(DoorStruct& Door)
	{
		for (std::int_fast32_t y{}; y < TextureSize; ++y)
		{
			const std::int_fast32_t TempY{ y * TextureSize };

			for (std::int_fast32_t SourceTextureX{}, x{ static_cast<std::int_fast32_t>(Door.CurrentOpenPercent / 1.59F) }; x < TextureSize; ++x, ++SourceTextureX)
			{
				Door.AnimTexture.Pixels[TempY + x] = DoorTypes[Door.DoorType].OriginalTexture.Pixels[TempY + SourceTextureX];
			}
		}
	}

	inline void OpenCloseDoors()
	{
		for (auto&& Door : Doors)
		{
			// Open door
			if (Door.IsOpenTriggered)
			{
				if (Door.CurrentOpenPercent < DoorTypes[Door.DoorType].MaximumOpenPercent)
				{
					Door.CurrentOpenPercent += DoorTypes[Door.DoorType].OpenCloseSpeed;
					ModifyDoorTexture(Door);
				}

				if (Door.CurrentOpenPercent >= DoorTypes[Door.DoorType].MaximumOpenPercent)
				{
					Door.IsOpen = true;
					Door.IsOpenTriggered = false;
					Door.IsCloseTriggered = true;
					Door.StayOpenCounter = DoorTypes[Door.DoorType].StayOpenTime;
					Door.CurrentOpenPercent = DoorTypes[Door.DoorType].MaximumOpenPercent;
					Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][static_cast<std::int_fast32_t>(Door.Pos.X)][static_cast<std::int_fast32_t>(Door.Pos.Y)] = 0;
				}
			}

			// Close door - but first check if door is not blocked!
			if (Door.IsCloseTriggered
				&& Game_EntityHandling::EntityMap[static_cast<std::int_fast32_t>(Door.Pos.X)][static_cast<std::int_fast32_t>(Door.Pos.Y)] == Game_EntityHandling::EntityTypes::Clear
				&& (std::abs(Player.Pos.X - Door.Pos.X) > FLT_EPSILON || std::abs(Player.Pos.Y - Door.Pos.Y) > FLT_EPSILON))
			{
				if (--Door.StayOpenCounter <= 0)
				{
					Door.IsOpen = false;

					if (Door.CurrentOpenPercent >= DoorTypes[Door.DoorType].OpenCloseSpeed)
					{
						if (!Door.OpenCloseAudioFlag)
						{
							PlayAudio(Door, DoorSounds::OpenCloseSound);
							Door.OpenCloseAudioFlag = true;
						}

						Door.CurrentOpenPercent -= DoorTypes[Door.DoorType].OpenCloseSpeed;
						ModifyDoorTexture(Door);
					}
				}

				if (Door.CurrentOpenPercent <= DoorTypes[Door.DoorType].MinimumOpenPercent)
				{
					Door.IsCloseTriggered = false;
					Door.OpenCloseAudioFlag = false;
					Door.CurrentOpenPercent = DoorTypes[Door.DoorType].MinimumOpenPercent;
					Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][static_cast<std::int_fast32_t>(Door.Pos.X)][static_cast<std::int_fast32_t>(Door.Pos.Y)] = INT_MAX;
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
