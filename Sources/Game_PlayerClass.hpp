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

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Game_Effects.hpp"

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

	std::vector<lwmf::MP3Player> Sounds{};
	lwmf::IntPointStruct FuturePos{};
	lwmf::FloatPointStruct Pos{};
	lwmf::FloatPointStruct Dir{};
	lwmf::FloatPointStruct StepWidth{};
	std::int_fast32_t SelectedWeapon{};
	std::int_fast32_t Hitpoints{};
	float MoveSpeed{};
	float CollisionDetectionFactor{};
	bool IsDead{};
};


inline void Game_PlayerClass::InitConfig()
{
	std::string INIFile{ LevelFolder };
	INIFile += std::to_string(SelectedLevel);
	INIFile += "/PlayerData/Config.ini";

	if (Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		MoveSpeed = lwmf::ReadINIValue<float>(INIFile, "GENERAL", "MoveSpeed");
		Hitpoints = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "Hitpoints");
		CollisionDetectionFactor = MoveSpeed + lwmf::ReadINIValue<float>(INIFile, "GENERAL", "CollisionDetectionWallDist");

		Pos = { lwmf::ReadINIValue<float>(INIFile, "POSITION", "PosX"), lwmf::ReadINIValue<float>(INIFile, "POSITION", "PosY") };
		Dir = { lwmf::ReadINIValue<float>(INIFile, "POSITION", "DirX"), lwmf::ReadINIValue<float>(INIFile, "POSITION", "DirY") };
		SelectedWeapon = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "WEAPON", "SelectedWeapon");
	}
}

inline void Game_PlayerClass::InitAudio()
{
	CloseAudio();
	Sounds.clear();
	Sounds.shrink_to_fit();

	std::string INIFile{ LevelFolder };
	INIFile += std::to_string(SelectedLevel);
	INIFile += "/PlayerData/Config.ini";

	if (Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		// Get Footsteps audio
		Sounds.emplace_back();
		Sounds[static_cast<std::int_fast32_t>(PlayerSounds::FootSteps)].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "FootStepsAudio"));

		// Get Hurt audio
		Sounds.emplace_back();
		Sounds[static_cast<std::int_fast32_t>(PlayerSounds::Hurt)].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "HurtAudio"));

		// Get DeathScream audio
		Sounds.emplace_back();
		Sounds[static_cast<std::int_fast32_t>(PlayerSounds::DeathScream)].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "DeathScreamAudio"));
	}
}

inline void Game_PlayerClass::HurtPlayer(const std::int_fast32_t DamageDealt)
{
	Hitpoints -= DamageDealt;

	if (Hitpoints > 0)
	{
		PlayAudio(PlayerSounds::Hurt);
		Game_Effects::StartBloodstainDrawing();
	}

	if (Hitpoints <= 0)
	{
		IsDead = true;
		PlayAudio(PlayerSounds::DeathScream);
	}
}

inline void Game_PlayerClass::PlayAudio(const PlayerSounds Sound)
{
	Sounds[static_cast<std::int_fast32_t>(Sound)].Play();
}

inline void Game_PlayerClass::CloseAudio()
{
	for (auto&& Sound : Sounds)
	{
		Sound.Close();
	}
}