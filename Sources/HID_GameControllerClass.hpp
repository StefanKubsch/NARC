/*
***********************************************
*                                             *
* HID_GameControllerClass.hpp                 *
*                                             *
* (c) 2017, 2018, 2019 Stefan Kubsch          *
***********************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <SDL.h>

#include "Tools_ErrorHandling.hpp"
#include "Tools_Curl.hpp"

class HID_GameControllerClass final
{
public:
	void Init();

	lwmf::IntPointStruct RightStickPos{};
	std::int_fast32_t JoystickDeadZone{};
	std::int_fast32_t RightStickValue{};
	float Sensitivity{};
	float RotationXLimit{};
};

inline void HID_GameControllerClass::Init()
{
	if (const std::string INIFile{ "./DATA/GameConfig/InputConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		JoystickDeadZone = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GAMECONTROLLER", "JoystickDeadZone");
		Sensitivity = lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "Sensitivity");
		RotationXLimit = lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "RotationXLimit");
		const std::string GameControllerDBURL{ lwmf::ReadINIValue <std::string>(INIFile, "GAMECONTROLLER", "GameControllerDBURL") };
		const std::string GameControllerDBFile{ lwmf::ReadINIValue <std::string>(INIFile, "GAMECONTROLLER", "GameControllerDBFile") };

		Tools_Curl::FetchFileFromURL(GameControllerDBURL, GameControllerDBFile);

		NARCLog.AddEntry("Searching and initializing gamecontroller...");

		if (SDL_NumJoysticks() > 0 && SDL_IsGameController(0) == SDL_TRUE)
		{
			NARCLog.AddEntry("Found a valid gamecontroller: ");

			if (SDL_GameController* Controller{ SDL_GameControllerOpen(0) }; Controller != nullptr)
			{
				NARCLog.AddEntry(std::string(SDL_GameControllerName(Controller)));

				if (Tools_ErrorHandling::CheckFileExistence(GameControllerDBFile, StopOnError))
				{
					NARCLog.AddEntry("Loading mapping from GameControllerDB file...");

					if (SDL_GameControllerAddMappingsFromFile(GameControllerDBFile.c_str()); SDL_GameControllerMapping(Controller) == nullptr)
					{
						NARCLog.LogErrorAndThrowException("No mapping found for: " + std::string(SDL_GameControllerName(Controller)));
					}

					SDL_GameControllerEventState(SDL_ENABLE);
				}
			}
			else
			{
				NARCLog.LogErrorAndThrowException("Gamecontroller init failed: " + std::string(SDL_GetError()));
			}
		}
		else
		{
			NARCLog.AddEntry("No gamecontroller found...");
		}
	}
}