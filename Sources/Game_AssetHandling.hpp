/*
******************************************
*                                        *
* Game_AssetHandling.hpp                 *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <SDL_mixer.h>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "SFX_SDL.hpp"
#include "GFX_ImageHandling.hpp"
#include "Game_DataStructures.hpp"

namespace Game_AssetHandling
{


	void InitAssets();
	void LoadWalkAnimTextures(std::int_fast32_t AssetIndex, const std::string& AssetTypeName);
	void LoadAdditionalAnimTextures(const std::string& AnimType, const std::string& AssetTypeName, std::vector<lwmf::TextureStruct>& AnimVector);
	void CloseAudio();

	//
	// Functions
	//

	inline void InitAssets()
	{
		EntityAssets.clear();
		EntityAssets.shrink_to_fit();

		std::int_fast32_t AssetFileIndex{};
		std::int_fast32_t AssetIndex{};

		while (true) //-V776
		{
			bool SkipAssetFlag{};

			if (const std::string EntityDataFile{ "./DATA/Level_" + std::to_string(SelectedLevel) + "/EntityData/" + std::to_string(AssetFileIndex) + ".ini" }; Tools_ErrorHandling::CheckFileExistence(EntityDataFile, ContinueOnError))
			{
				const std::string AssetTypeName{ lwmf::ReadINIValue<std::string>(EntityDataFile, "ENTITY", "EntityTypeName") };

				// If asset type was already loaded, skip this...
				for (const auto& Asset : EntityAssets)
				{
					if (Asset.Name == AssetTypeName)
					{
						SkipAssetFlag = true;
						break;
					}
				}

				if (const std::string INIFile{ "./DATA/Assets/" + AssetTypeName + "/AssetData.ini" }; !SkipAssetFlag && Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
				{
					EntityAssets.emplace_back();
					EntityAssets[AssetIndex].Number = AssetIndex;
					EntityAssets[AssetIndex].Name = AssetTypeName;

					//
					// Get GFX
					//

					LoadWalkAnimTextures(AssetIndex, AssetTypeName);
					LoadAdditionalAnimTextures("Attack", AssetTypeName, EntityAssets[AssetIndex].AttackTextures);
					LoadAdditionalAnimTextures("Kill", AssetTypeName, EntityAssets[AssetIndex].KillTextures);

					//
					// Get SFX
					//

					if (const std::string AssetType{ lwmf::ReadINIValue<std::string>(INIFile, "GENERAL", "AssetType") }; AssetType == "AmmoBox")
					{
						// Get Pickup audio
						EntityAssets[AssetIndex].Sounds.emplace_back(SFX_SDL::LoadAudioFile(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "AmmoPickup")));
					}
					else if (AssetType == "Enemy" || AssetType == "Turret")
					{
						// Get KillSound audio
						EntityAssets[AssetIndex].Sounds.emplace_back(SFX_SDL::LoadAudioFile(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "KillSound")));

						// Get AttackSound audio
						EntityAssets[AssetIndex].Sounds.emplace_back(SFX_SDL::LoadAudioFile(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "AttackSound")));
					}

					++AssetIndex;
				}

				++AssetFileIndex;
			}
			else
			{
				break;
			}
		}
	}

	inline void LoadWalkAnimTextures(const std::int_fast32_t AssetIndex, const std::string& AssetTypeName)
	{
		std::int_fast32_t DirectionIndex{};

		while (true)
		{
			if (const std::string Path{ "./GFX/Entities/" + std::to_string(EntitySize) + "/" + AssetTypeName + "/" + std::to_string(DirectionIndex) }; Tools_ErrorHandling::CheckFolderExistence(Path, ContinueOnError))
			{
				EntityAssets[AssetIndex].WalkingTextures.emplace_back();

				std::int_fast32_t TextureIndex{};

				while (true)
				{
					if (const std::string Texture{ Path + "/" + std::to_string(TextureIndex) + ".png" }; Tools_ErrorHandling::CheckFileExistence(Texture, ContinueOnError))
					{
						EntityAssets[AssetIndex].WalkingTextures[DirectionIndex].emplace_back(GFX_ImageHandling::ImportTexture(Texture, EntitySize));
						++TextureIndex;
					}
					else
					{
						break;
					}
				}

				++DirectionIndex;
			}
			else
			{
				break;
			}
		}
	}

	inline void LoadAdditionalAnimTextures(const std::string& AnimType, const std::string& AssetTypeName, std::vector<lwmf::TextureStruct>& AnimVector)
	{
		std::int_fast32_t TextureIndex{};

		while (true)
		{
			if (const std::string Texture{ "./GFX/Entities/" + std::to_string(EntitySize) + "/" + AssetTypeName + "/" + AnimType + "/" + std::to_string(TextureIndex) +".png" }; Tools_ErrorHandling::CheckFileExistence(Texture, ContinueOnError))
			{
				AnimVector.emplace_back(GFX_ImageHandling::ImportTexture(Texture, EntitySize));
				++TextureIndex;
			}
			else
			{
				break;
			}
		}
	}

	inline void CloseAudio()
	{
		for (const auto& Asset : EntityAssets)
		{
			for (auto&& Sound : Asset.Sounds)
			{
				Mix_FreeChunk(Sound);
			}
		}
	}


} // namespace Game_AssetHandling