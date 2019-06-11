/*
******************************************
*                                        *
* Game_PlayerClass.hpp                   *
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

class Game_PlayerClass final
{
public:
	enum class PlayerSounds : std::int_fast32_t
	{
		FootSteps,
		Hurt,
		DeathScream
	};

	void InitConfig();
	void InitAudio();
	void HurtPlayer(std::int_fast32_t DamageDealt);
	void PlayAudio(PlayerSounds Sound);
	void CloseAudio();

	lwmf::IntPointStruct FuturePos{};
	lwmf::FloatPointStruct Pos{};
	lwmf::FloatPointStruct Dir{};
	lwmf::FloatPointStruct StepWidth{};
	std::int_fast32_t SelectedWeapon{};
	std::int_fast32_t Hitpoints{};
	float MoveSpeed{};
	float CollisionDetectionFactor{};
	bool IsDead{};

private:
	std::vector<Mix_Chunk*> Sounds;
	std::int_fast32_t FootStepsAudioChannel{};
};

inline void Game_PlayerClass::InitConfig()
{
	if (const std::string INIFile{ "./DATA/Level_" + std::to_string(SelectedLevel) + "/PlayerData/Config.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		MoveSpeed = lwmf::ReadINIValue<float>(INIFile, "GENERAL", "MoveSpeed");
		Hitpoints = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "Hitpoints");
		CollisionDetectionFactor = MoveSpeed + lwmf::ReadINIValue<float>(INIFile, "GENERAL", "CollisionDetectionWallDist");
		Pos.X = lwmf::ReadINIValue<float>(INIFile, "POSITION", "PosX");
		Pos.Y = lwmf::ReadINIValue<float>(INIFile, "POSITION", "PosY");
		Dir.X = lwmf::ReadINIValue<float>(INIFile, "POSITION", "DirX");
		Dir.Y = lwmf::ReadINIValue<float>(INIFile, "POSITION", "DirY");
		SelectedWeapon = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "WEAPON", "SelectedWeapon");
	}
}

inline void Game_PlayerClass::InitAudio()
{
	Sounds.clear();
	Sounds.shrink_to_fit();

	if (const std::string INIFile{ "./DATA/Level_" + std::to_string(SelectedLevel) + "/PlayerData/Config.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		// Get Footsteps audio
		Sounds.emplace_back(SFX_SDL::LoadAudioFile(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "FootStepsAudio")));
		FootStepsAudioChannel = 0;

		// Get Hurt audio
		Sounds.emplace_back(SFX_SDL::LoadAudioFile(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "HurtAudio")));

		// Get DeathScream audio
		Sounds.emplace_back(SFX_SDL::LoadAudioFile(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "DeathScreamAudio")));
	}
}

inline void Game_PlayerClass::HurtPlayer(const std::int_fast32_t DamageDealt)
{
	Hitpoints -= DamageDealt;

	if (Hitpoints > 0)
	{
		PlayAudio(PlayerSounds::Hurt);
	}

	if (Hitpoints <= 0)
	{
		IsDead = true;
		PlayAudio(PlayerSounds::DeathScream);
	}
}

inline void Game_PlayerClass::PlayAudio(const PlayerSounds Sound)
{
	if (Sound == PlayerSounds::FootSteps && Mix_Playing(FootStepsAudioChannel) == 0)
	{
		FootStepsAudioChannel = Mix_PlayChannel(-1, Sounds[static_cast<std::int_fast32_t>(Sound)], 0);
	}
	else if (Sound != PlayerSounds::FootSteps)
	{
		Mix_PlayChannel(-1, Sounds[static_cast<std::int_fast32_t>(Sound)], 0);
	}
}

inline void Game_PlayerClass::CloseAudio()
{
	for (auto&& Sound : Sounds)
	{
		Mix_FreeChunk(Sound);
	}
}

