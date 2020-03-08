/*
*****************************************
*                                       *
* Game_Doors.hpp                        *
*                                       *
* (c) 2017 - 2020 Stefan Kubsch         *
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


	enum class DoorSounds : std::int_fast32_t
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

	constexpr float MaximumOpenPercentLowerLimit{ 0.0F };
	constexpr float MaximumOpenPercentUpperLimit{ 100.0F };
	constexpr float MinimumOpenPercentLowerLimit{ 0.0F };
	constexpr float MinimumOpenPercentUpperLimit{ 100.0F };

	//
	// Functions
	//

	inline void InitDoorAssets()
	{
		CloseAudio();

		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init door assets...");

		DoorTypes.clear();
		DoorTypes.shrink_to_fit();
		DoorTypes.resize(1);

		// We start our DoorTypes counting at "1", since in the map definition it has to be greater zero!
		std::int_fast32_t Index{ 1 };

		while (true)
		{
			std::string INIFile{ AssetsDoorsFolder };
			INIFile += "Door_";
			INIFile += std::to_string(Index);
			INIFile += "_Data.ini";

			if (Tools_ErrorHandling::CheckFileExistence(INIFile, ContinueOnError))
			{
				DoorTypes.emplace_back();

				lwmf::LoadPNG(DoorTypes[Index].OriginalTexture, lwmf::ReadINIValue<std::string>(INIFile, "TEXTURE", "DoorTexture"));

				DoorTypes[Index].Sounds.emplace_back();
				DoorTypes[Index].Sounds[static_cast<std::int_fast32_t>(DoorSounds::OpenCloseSound)].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "OpenCloseSound"));
				DoorTypes[Index].OpenCloseSpeed = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "OpenCloseSpeed");
				DoorTypes[Index].StayOpenTime = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "StayOpenTime") * static_cast<std::int_fast32_t>(FrameLock);
				DoorTypes[Index].MaximumOpenPercent = lwmf::ReadINIValue<float>(INIFile, "GENERAL", "MaximumOpenPercent");
				DoorTypes[Index].MinimumOpenPercent = lwmf::ReadINIValue<float>(INIFile, "GENERAL", "MinimumOpenPercent");

				Tools_ErrorHandling::CheckAndClampRange(DoorTypes[Index].MaximumOpenPercent, MaximumOpenPercentLowerLimit, MaximumOpenPercentUpperLimit, __FILENAME__, "MaximumOpenPercent");
				Tools_ErrorHandling::CheckAndClampRange(DoorTypes[Index].MinimumOpenPercent, MinimumOpenPercentLowerLimit, MinimumOpenPercentUpperLimit, __FILENAME__, "MinimumOpenPercent");

				++Index;
			}
			else
			{
				NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "No more doortype data found.");
				break;
			}
		}
	}

	inline void InitDoors()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init doors...");

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

					Doors[Index].AnimTexture = DoorTypes[FoundDoorType].OriginalTexture;
					Doors[Index].Pos = { static_cast<float>(MapPosX), static_cast<float>(MapPosY) };
					Doors[Index].State = DoorStruct::States::Closed;
					Doors[Index].DoorType = FoundDoorType;
					Doors[Index].Number = Index;
					Doors[Index].CurrentOpenPercent = DoorTypes[Doors[Index].DoorType].MinimumOpenPercent;

					ModifyDoorTexture(Doors[Index]);

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
			if (Door.State == DoorStruct::States::Closed && (std::abs(Door.Pos.X - Player.FuturePos.X) < FLT_EPSILON && std::abs(Door.Pos.Y - Player.FuturePos.Y) < FLT_EPSILON))
			{
				Door.State = DoorStruct::States::Triggered;
				PlayAudio(Door, DoorSounds::OpenCloseSound);
				break;
			}
		}
	}

	inline void ModifyDoorTexture(DoorStruct& Door)
	{
		// TextureOffset is not correct - works only when MaximumOpenPercent = 100% !
		static const float TextureOffset{ 100.0F / static_cast<float>(TextureSize) };
		const std::int_fast32_t OpenPercent{ static_cast<std::int_fast32_t>(Door.CurrentOpenPercent / TextureOffset) };

		for (std::int_fast32_t y{}; y < TextureSize; ++y)
		{
			const std::int_fast32_t TempY{ y * TextureSize };
			const auto SourceY{ DoorTypes[Door.DoorType].OriginalTexture.Pixels.begin() + TempY };
			std::copy(SourceY, SourceY + TextureSize - OpenPercent, Door.AnimTexture.Pixels.begin() + TempY + OpenPercent);
		}
	}

	inline void OpenCloseDoors()
	{
		for (auto&& Door : Doors)
		{
			// Open door
			if (Door.State == DoorStruct::States::Triggered)
			{
				if (Door.CurrentOpenPercent < DoorTypes[Door.DoorType].MaximumOpenPercent)
				{
					Door.CurrentOpenPercent += DoorTypes[Door.DoorType].OpenCloseSpeed;
					ModifyDoorTexture(Door);
				}

				if (Door.CurrentOpenPercent >= DoorTypes[Door.DoorType].MaximumOpenPercent)
				{
					Door.State = DoorStruct::States::Open;
					Door.StayOpenCounter = DoorTypes[Door.DoorType].StayOpenTime;
					Door.CurrentOpenPercent = DoorTypes[Door.DoorType].MaximumOpenPercent;
					Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][static_cast<std::int_fast32_t>(Door.Pos.X)][static_cast<std::int_fast32_t>(Door.Pos.Y)] = 0;
				}
			}

			// Close door - but first check if door is not blocked!
			if (Door.State == DoorStruct::States::Open
				&& Game_EntityHandling::EntityMap[static_cast<std::int_fast32_t>(Door.Pos.X)][static_cast<std::int_fast32_t>(Door.Pos.Y)] == EntityTypes::Clear
				&& (std::abs(Player.Pos.X - Door.Pos.X) > FLT_EPSILON || std::abs(Player.Pos.Y - Door.Pos.Y) > FLT_EPSILON))
			{
				if (--Door.StayOpenCounter <= 0)
				{
					if (Door.CurrentOpenPercent >= DoorTypes[Door.DoorType].OpenCloseSpeed)
					{
						if (!Door.CloseAudioFlag)
						{
							PlayAudio(Door, DoorSounds::OpenCloseSound);
							Door.CloseAudioFlag = true;
						}

						Door.CurrentOpenPercent -= DoorTypes[Door.DoorType].OpenCloseSpeed;
						ModifyDoorTexture(Door);
					}
				}

				if (Door.CurrentOpenPercent <= DoorTypes[Door.DoorType].MinimumOpenPercent)
				{
					Door.State = DoorStruct::States::Closed;
					Door.CloseAudioFlag = false;
					Door.CurrentOpenPercent = DoorTypes[Door.DoorType].MinimumOpenPercent;
					Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][static_cast<std::int_fast32_t>(Door.Pos.X)][static_cast<std::int_fast32_t>(Door.Pos.Y)] = INT_MAX;
				}
			}
		}
	}

	inline void PlayAudio(const DoorStruct& Door, const DoorSounds Sound)
	{
		DoorTypes[Door.DoorType].Sounds[static_cast<std::int_fast32_t>(Sound)].Play();
	}

	inline void CloseAudio()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Close door audio...");

		for (auto&& DoorType : DoorTypes)
		{
			for (auto&& Sound : DoorType.Sounds)
			{
				Sound.Close();
			}
		}
	}


} // namespace Game_Doors
