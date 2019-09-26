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
	void OpenCloseDoors();
	void PlayAudio(const DoorStruct& Door, DoorSounds Sound);
	void CloseAudio();

	//
	// Functions
	//

	inline void InitDoorAssets()
	{
		DoorTypes.clear();
		DoorTypes.shrink_to_fit();

		// We start our DoorTypes counting at "1", since in the map definition it has to be greater zero!
		DoorTypes.resize(1);
		std::int_fast32_t Index{ 1 };

		while (true)
		{
			if (const std::string INIFile{ "./DATA/Assets_Doors/Door_" + std::to_string(Index) + "_Data.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, ContinueOnError))
			{
				DoorTypes.emplace_back();

				lwmf::LoadPNG(DoorTypes[Index].OriginalTexture, lwmf::ReadINIValue<std::string>(INIFile, "TEXTURE", "DoorTexture"));

				DoorTypes[Index].Sounds.emplace_back();
				DoorTypes[Index].Sounds[static_cast<std::int_fast32_t>(DoorSounds::OpenCloseSound)].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "OpenCloseSound"));

				DoorTypes[Index].OpenCloseWidth = TextureSize;
				DoorTypes[Index].OpenCloseSpeed = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "OpenCloseSpeed");
				DoorTypes[Index].StayOpenTime = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "StayOpenTime") * static_cast<std::int_fast32_t>(FrameLock);

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
					Doors[Index].Pos = { MapPosX, MapPosY };

					++Index;
				}
			}
		}
	}

	inline void TriggerDoor()
	{
		for (auto&& Door : Doors)
		{
			if (!Door.IsOpen && (Door.Pos.X == Player.FuturePos.X && Door.Pos.Y == Player.FuturePos.Y))
			{
				Door.IsOpenTriggered = true;
				Door.OpenCloseCounter = 0;
				PlayAudio(Door, DoorSounds::OpenCloseSound);
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
				if (Door.OpenCloseCounter < DoorTypes[Door.DoorType].OpenCloseWidth)
				{
					Door.OpenCloseCounter += DoorTypes[Door.DoorType].OpenCloseSpeed;

					// TODO(Stefan): Of course, you have to see what´s behind the door when opening/closing...Rendering needs to be changed for that!
					std::fill(Door.AnimTexture.Pixels.begin(), Door.AnimTexture.Pixels.end(), lwmf::AMask);

					for (std::int_fast32_t SourceTextureX{}, x{ Door.OpenCloseCounter }; x < DoorTypes[Door.DoorType].OpenCloseWidth; ++x, ++SourceTextureX)
					{
						for (std::int_fast32_t y{}; y < TextureSize; ++y)
						{
							Door.AnimTexture.Pixels[y * TextureSize + x] = DoorTypes[Door.DoorType].OriginalTexture.Pixels[y * TextureSize + SourceTextureX];
						}
					}
				}

				if (Door.OpenCloseCounter >= DoorTypes[Door.DoorType].OpenCloseWidth)
				{
					Door.IsOpen = true;
					Door.IsOpenTriggered = false;
					Door.IsCloseTriggered = true;
					Door.StayOpenCounter = DoorTypes[Door.DoorType].StayOpenTime;
					Door.OpenCloseCounter = DoorTypes[Door.DoorType].OpenCloseWidth;
				}
			}

			// Close door - but first check if door is not blocked!
			if (Door.IsCloseTriggered
				&& Game_EntityHandling::EntityMap[Door.Pos.X][Door.Pos.Y] == Game_EntityHandling::EntityTypes::Clear
				&& (static_cast<std::int_fast32_t>(Player.Pos.X) != Door.Pos.X || static_cast<std::int_fast32_t>(Player.Pos.Y) != Door.Pos.Y))
			{
				if (--Door.StayOpenCounter <= 0)
				{
					Door.IsOpen = false;

					if (Door.OpenCloseCounter >= DoorTypes[Door.DoorType].OpenCloseSpeed)
					{
						if (!Door.OpenCloseAudioFlag)
						{
							PlayAudio(Door, DoorSounds::OpenCloseSound);
							Door.OpenCloseAudioFlag = true;
						}

						Door.OpenCloseCounter -= DoorTypes[Door.DoorType].OpenCloseSpeed;

						for (std::int_fast32_t SourceTextureX{}, x{ Door.OpenCloseCounter }; x < DoorTypes[Door.DoorType].OpenCloseWidth; ++x, ++SourceTextureX)
						{
							for (std::int_fast32_t y{}; y < TextureSize; ++y)
							{
								Door.AnimTexture.Pixels[y * TextureSize + x] = DoorTypes[Door.DoorType].OriginalTexture.Pixels[y * TextureSize + SourceTextureX];
							}
						}
					}
				}

				if (Door.OpenCloseCounter <= 0)
				{
					Door.IsCloseTriggered = false;
					Door.OpenCloseAudioFlag = false;
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
