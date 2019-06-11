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
#include <SDL_mixer.h>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "SFX_SDL.hpp"
#include "Game_DataStructures.hpp"
#include "Game_LevelHandling.hpp"
#include "Game_EntityHandling.hpp"

namespace Game_Doors
{


	enum class DoorSounds : std::int_fast32_t
	{
		OpenClose
	};

	void Init();
	void TriggerDoor();
	void OpenCloseDoors();
	void PlayAudio(const DoorStruct& Door, DoorSounds DoorSound);
	void CloseAudio();

	//
	// Functions
	//

	inline void Init()
	{
		Doors.clear();
		Doors.shrink_to_fit();

		if (const std::string INIFile{ "./DATA/Level_" + std::to_string(SelectedLevel) + "/LevelData/DoorConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			for (std::int_fast32_t Index{}, MapPosX{}; MapPosX < Game_LevelHandling::LevelMapWidth; ++MapPosX)
			{
				for (std::int_fast32_t MapPosY{}; MapPosY < Game_LevelHandling::LevelMapHeight; ++MapPosY)
				{
					if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Door)][MapPosX][MapPosY] > 0)
					{
						Doors.emplace_back();

						Doors[Index].Number = Index;
						Doors[Index].Pos.X = MapPosX;
						Doors[Index].Pos.Y = MapPosY;
						Doors[Index].OriginalTexture = Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Door)][MapPosX][MapPosY];
						Doors[Index].OpenCloseWidth = TextureSize;
						Doors[Index].OpenCloseSpeed = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "OpenCloseSpeed");
						Doors[Index].StayOpenTime = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "StayOpenTime") * static_cast<std::int_fast32_t>(FrameLock);
						Doors[Index].AnimTexture = Game_LevelHandling::LevelTextures[Doors[Index].OriginalTexture];

						Doors[Index].Sounds.emplace_back(SFX_SDL::LoadAudioFile(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "OpenCloseSound")));

						++Index;
					}
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
				PlayAudio(Door, DoorSounds::OpenClose);
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
				if (Door.OpenCloseCounter < Door.OpenCloseWidth)
				{
					Door.OpenCloseCounter += Door.OpenCloseSpeed;

					// TODO(Stefan): Of course, you have to see what�s behind the door when opening/closing...Rendering needs to be changed for that!
					std::fill(Door.AnimTexture.Pixels.begin(), Door.AnimTexture.Pixels.end(), lwmf::AMask);

					for (std::int_fast32_t SourceTextureX{}, x{ Door.OpenCloseCounter }; x < Door.OpenCloseWidth; ++x, ++SourceTextureX)
					{
						for (std::int_fast32_t y{}; y < TextureSize; ++y)
						{
							Door.AnimTexture.Pixels[y * TextureSize + x] = Game_LevelHandling::LevelTextures[Door.OriginalTexture].Pixels[y * TextureSize + SourceTextureX];
						}
					}
				}

				if (Door.OpenCloseCounter >= Door.OpenCloseWidth)
				{
					Door.IsOpen = true;
					Door.IsOpenTriggered = false;
					Door.IsCloseTriggered = true;
					Door.StayOpenCounter = Door.StayOpenTime;
					Door.OpenCloseCounter = Door.OpenCloseWidth;
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

					if (Door.OpenCloseCounter >= Door.OpenCloseSpeed)
					{
						if (!Door.OpenCloseAudioFlag)
						{
							PlayAudio(Door, DoorSounds::OpenClose);
							Door.OpenCloseAudioFlag = true;
						}

						Door.OpenCloseCounter -= Door.OpenCloseSpeed;

						for (std::int_fast32_t SourceTextureX{}, x{ Door.OpenCloseCounter }; x < Door.OpenCloseWidth; ++x, ++SourceTextureX)
						{
							for (std::int_fast32_t y{}; y < TextureSize; ++y)
							{
								Door.AnimTexture.Pixels[y * TextureSize + x] = Game_LevelHandling::LevelTextures[Door.OriginalTexture].Pixels[y * TextureSize + SourceTextureX];
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

	inline void PlayAudio(const DoorStruct& Door, const DoorSounds DoorSound)
	{
		Mix_PlayChannel(-1, Door.Sounds[static_cast<std::int_fast32_t>(DoorSound)], 0);
	}

	inline void CloseAudio()
	{
		for (const auto& Door : Doors)
		{
			for (auto&& Sound : Door.Sounds)
			{
				Mix_FreeChunk(Sound);
			}
		}
	}


} // namespace Game_Doors
