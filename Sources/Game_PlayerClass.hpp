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
	if (const std::string INIFile{ "./DATA/Level_" + std::to_string(SelectedLevel) + "/PlayerData/Config.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		// Get Footsteps audio
		lwmf::LoadMP3(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "FootStepsAudio"), "FootSteps");

		// Get Hurt audio
		lwmf::LoadMP3(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "HurtAudio"), "Hurt");

		// Get DeathScream audio
		lwmf::LoadMP3(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "DeathScreamAudio"), "DeathScream");
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
	switch (Sound)
	{
		case PlayerSounds::FootSteps:
		{
			lwmf::PlayMP3("FootSteps", lwmf::MainWindow, lwmf::AudioPlayModes::NOTIFY);
			break;
		}
		case PlayerSounds::Hurt:
		{
			lwmf::PlayMP3("Hurt", lwmf::MainWindow, lwmf::AudioPlayModes::FROMSTART);
			break;
		}
		case PlayerSounds::DeathScream:
		{
			lwmf::PlayMP3("DeathScream", lwmf::MainWindow, lwmf::AudioPlayModes::FROMSTART);
			break;
		}
		default: {}
	}
}

inline void Game_PlayerClass::CloseAudio()
{
	lwmf::CloseMP3("FootSteps");
	lwmf::CloseMP3("Hurt");
	lwmf::CloseMP3("DeathScream");
}

