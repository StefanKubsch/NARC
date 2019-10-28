/*
*****************************************
*                                       *
* Game_LevelHandling.hpp                *
*                                       *
* (c) 2017, 2018, 2019 Stefan Kubsch    *
*****************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "GFX_ImageHandling.hpp"
#include "GFX_LightingClass.hpp"

namespace Game_LevelHandling
{


	enum class LevelMapLayers : std::int_fast32_t
	{
		Floor,
		Wall,
		Ceiling,
		Door,
		Counter
	};

	void InitConfig();
	void ReadMapDataFile(const std::string& FileName, std::vector<std::vector<std::vector<std::int_fast32_t>>>& LevelMapVector, LevelMapLayers LevelMapLayer);
	void InitMapData();
	void InitLights();
	void InitTextures();
	void InitBackgroundMusic();
	void PlayBackgroundMusic();
	void CloseAudio();

	//
	// Variables and constants
	//

	inline std::vector<std::vector<std::vector<std::int_fast32_t>>> LevelMap{};
	inline std::vector<lwmf::TextureStruct> LevelTextures{};

	inline std::vector<GFX_LightingClass> StaticLights{};
	inline std::vector<lwmf::MP3Player> BackgroundMusic;

	// Variables used for map dimensions (used for Level*Map and EntityMap)
	inline std::int_fast32_t LevelMapWidth{};
	inline std::int_fast32_t LevelMapHeight{};

	inline bool LightingFlag{};
	inline bool BackgroundMusicEnabled{};

	//
	// Functions
	//

	inline void InitConfig()
	{
		if (const std::string INIFile{ LevelFolder + std::to_string(SelectedLevel) + "/LevelData/Config.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			LightingFlag = lwmf::ReadINIValue<bool>(INIFile, "GENERAL", "Lighting");
		}
	}

	inline void ReadMapDataFile(const std::string& FileName, std::vector<std::vector<std::vector<std::int_fast32_t>>>& LevelMapVector, const LevelMapLayers LevelMapLayer)
	{
		if (Tools_ErrorHandling::CheckFileExistence(FileName, StopOnError))
		{
			std::ifstream LevelMapDataFile(FileName, std::ios::in);
			std::vector<std::int_fast32_t> TempVectorCeiling{};
			std::string Line;

			while (std::getline(LevelMapDataFile, Line))
			{
				std::istringstream Stream(Line);
				std::vector<std::int_fast32_t> TempVector{};
				std::int_fast32_t TempInt{};
				char Delimiter{ '\0' };

				while (Stream >> TempInt)
				{
					TempVector.emplace_back(TempInt);
					Stream >> Delimiter;
				}

				LevelMapVector[static_cast<std::int_fast32_t>(LevelMapLayer)].emplace_back(TempVector);
				TempVectorCeiling = std::move(TempVector);
			}

			// double the last line for ceiling only to prevent a bad behaviour concerning lighting...
			if (LevelMapLayer == LevelMapLayers::Ceiling)
			{
				LevelMapVector[static_cast<std::int_fast32_t>(LevelMapLayer)].emplace_back(TempVectorCeiling);
			}
		}
	}

	inline void InitMapData()
	{
		LevelMap.clear();
		LevelMap.shrink_to_fit();
		LevelMap.resize(static_cast<std::int_fast32_t>(LevelMapLayers::Counter));

		const std::string LevelPath{ LevelFolder + std::to_string(SelectedLevel) + "/LevelData/" };

		ReadMapDataFile(LevelPath + "MapFloorData.conf", LevelMap, LevelMapLayers::Floor);
		ReadMapDataFile(LevelPath + "MapWallData.conf", LevelMap, LevelMapLayers::Wall);
		ReadMapDataFile(LevelPath + "MapCeilingData.conf", LevelMap, LevelMapLayers::Ceiling);
		ReadMapDataFile(LevelPath + "MapDoorData.conf", LevelMap, LevelMapLayers::Door);

		LevelMapWidth = static_cast<std::int_fast32_t>(LevelMap[static_cast<std::int_fast32_t>(LevelMapLayers::Wall)].size());
		LevelMapHeight = static_cast<std::int_fast32_t>(LevelMap[static_cast<std::int_fast32_t>(LevelMapLayers::Wall)][0].size());
	}


	inline void InitLights()
	{
		StaticLights.clear();
		StaticLights.shrink_to_fit();

		if (LightingFlag)
		{
			if (const std::string FileName{ LevelFolder + std::to_string(SelectedLevel) + "/LevelData/StaticLightsData.conf" }; Tools_ErrorHandling::CheckFileExistence(FileName, StopOnError))
			{
				std::ifstream StaticLightsDataFile(FileName, std::ios::in);

				std::int_fast32_t Location{};
				lwmf::FloatPointStruct Pos{};
				float Radius{};
				float Intensity{};

				while (StaticLightsDataFile >> Pos.X >> Pos.Y >> Location >> Radius >> Intensity)
				{
					StaticLights.emplace_back(Pos.X, Pos.Y, Location, Radius, Intensity);
				}
			}
		}
	}

	inline void InitTextures()
	{
		LevelTextures.clear();
		LevelTextures.shrink_to_fit();

		if (const std::string FileName{ LevelFolder + std::to_string(SelectedLevel) + "/LevelData/TexturesData.conf" }; Tools_ErrorHandling::CheckFileExistence(FileName, StopOnError))
		{
			std::ifstream LevelTexturesDataFile(FileName, std::ios::in);
			std::string Line;

			while (std::getline(LevelTexturesDataFile, Line))
			{
				LevelTextures.emplace_back(GFX_ImageHandling::ImportTexture("./GFX/LevelTextures/" + std::to_string(TextureSize) + "/" + Line, TextureSize));
			}
		}
	}

	inline void InitBackgroundMusic()
	{
		BackgroundMusic.clear();
		BackgroundMusic.shrink_to_fit();

		if (const std::string INIFile{ LevelFolder + std::to_string(SelectedLevel) + "/LevelData/Config.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			BackgroundMusicEnabled = lwmf::ReadINIValue<bool>(INIFile, "AUDIO", "BackgroundMusicEnabled");

			if (BackgroundMusicEnabled)
			{
				if (const std::string AudioFileName{ lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "BackgroundMusic") }; Tools_ErrorHandling::CheckFileExistence(AudioFileName, StopOnError))
				{
					BackgroundMusic.emplace_back();
					BackgroundMusic[0].Load(AudioFileName);
				}
			}
		}
	}

	inline void PlayBackgroundMusic()
	{
		if (BackgroundMusicEnabled)
		{
			BackgroundMusic[0].Play();
		}
	}

	inline void CloseAudio()
	{
		for (auto&& Sound : BackgroundMusic)
		{
			Sound.Close();
		}
	}


} // namespace Game_LevelHandling