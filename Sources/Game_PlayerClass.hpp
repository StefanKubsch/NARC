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
#include "fmt/format.h"

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"
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

	PointInt FuturePos{};
	PointFloat Pos{};
	PointFloat Dir{};
	PointFloat StepWidth{};
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
	if (const std::string INIFile{ fmt::format("./DATA/Level_{}/PlayerData/Config.ini", SelectedLevel) }; Tools_ErrorHandling::CheckFileExistence(INIFile, ShowMessage, StopOnError))
	{
		MoveSpeed = Tools_INIFile::ReadValue<float>(INIFile, "GENERAL", "MoveSpeed");
		Hitpoints = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "GENERAL", "Hitpoints");
		CollisionDetectionFactor = MoveSpeed + Tools_INIFile::ReadValue<float>(INIFile, "GENERAL", "CollisionDetectionWallDist");
		Pos.X = Tools_INIFile::ReadValue<float>(INIFile, "POSITION", "PosX");
		Pos.Y = Tools_INIFile::ReadValue<float>(INIFile, "POSITION", "PosY");
		Dir.X = Tools_INIFile::ReadValue<float>(INIFile, "POSITION", "DirX");
		Dir.Y = Tools_INIFile::ReadValue<float>(INIFile, "POSITION", "DirY");
		SelectedWeapon = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "WEAPON", "SelectedWeapon");
	}
}

inline void Game_PlayerClass::InitAudio()
{
	Sounds.clear();
	Sounds.shrink_to_fit();

	if (const std::string INIFile{ fmt::format("./DATA/Level_{}/PlayerData/Config.ini", SelectedLevel) }; Tools_ErrorHandling::CheckFileExistence(INIFile, ShowMessage, StopOnError))
	{
		// Get Footsteps audio
		Sounds.emplace_back(SFX_SDL::LoadAudioFile(Tools_INIFile::ReadValue<std::string>(INIFile, "AUDIO", "FootStepsAudio")));
		FootStepsAudioChannel = 0;

		// Get Hurt audio
		Sounds.emplace_back(SFX_SDL::LoadAudioFile(Tools_INIFile::ReadValue<std::string>(INIFile, "AUDIO", "HurtAudio")));

		// Get DeathScream audio
		Sounds.emplace_back(SFX_SDL::LoadAudioFile(Tools_INIFile::ReadValue<std::string>(INIFile, "AUDIO", "DeathScreamAudio")));
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

