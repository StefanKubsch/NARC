/*
***********************************************
*                                             *
* HID_GameControllerClass.hpp                 *
*                                             *
* (c) 2017, 2018, 2019 Stefan Kubsch          *
***********************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <cstdint>
#include <string>
#include <SDL.h>
#include "fmt/format.h"

#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"
#include "Tools_Curl.hpp"

class HID_GameControllerClass final
{
public:
	void Init();

	SDL_Point RightStickPos{};
	std::int_fast32_t JoystickDeadZone{};
	std::int_fast32_t RightStickValue{};
	float Sensitivity{};
	float RotationXLimit{};
};

inline void HID_GameControllerClass::Init()
{
	if (const std::string INIFile{ "./DATA/GameConfig/InputConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, ShowMessage, StopOnError))
	{
		JoystickDeadZone = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "GAMECONTROLLER", "JoystickDeadZone");
		Sensitivity = Tools_INIFile::ReadValue<float>(INIFile, "GAMECONTROLLER", "Sensitivity");
		RotationXLimit = Tools_INIFile::ReadValue<float>(INIFile, "GAMECONTROLLER", "RotationXLimit");
		const std::string GameControllerDBURL = Tools_INIFile::ReadValue <std::string>(INIFile, "GAMECONTROLLER", "GameControllerDBURL");
		const std::string GameControllerDBFile = Tools_INIFile::ReadValue <std::string>(INIFile, "GAMECONTROLLER", "GameControllerDBFile");

		Tools_Curl::FetchFileFromURL(GameControllerDBURL, GameControllerDBFile);

		Tools_Console::DisplayText(BRIGHT_MAGENTA, "Searching and initializing gamecontroller...\n");

		if (SDL_NumJoysticks() > 0 &&  SDL_IsGameController(0) == SDL_TRUE)
		{
			Tools_Console::DisplayText(BRIGHT_GREEN, "Found a valid gamecontroller: ");

			if (SDL_GameController* Controller{ SDL_GameControllerOpen(0) }; Controller != nullptr)
			{
				Tools_Console::DisplayText(BRIGHT_GREEN, fmt::format("{}\n", SDL_GameControllerName(Controller)));

				if (Tools_ErrorHandling::CheckFileExistence(GameControllerDBFile, ShowMessage, StopOnError))
				{
					Tools_Console::DisplayText(BRIGHT_MAGENTA, "Loading mapping from GameControllerDB file...");

					if (SDL_GameControllerAddMappingsFromFile(GameControllerDBFile.c_str()); SDL_GameControllerMapping(Controller) == nullptr)
					{
						Tools_ErrorHandling::DisplayError(fmt::format("No mapping for {} found.", SDL_GameControllerName(Controller)));
					}

					SDL_GameControllerEventState(SDL_ENABLE);
					Tools_ErrorHandling::DisplayOK();
					fmt::print("\n");
				}
			}
			else
			{
				Tools_ErrorHandling::DisplayError(fmt::format("Gamecontroller init failed: {}", SDL_GetError()));
			}
		}
		else
		{
			Tools_Console::DisplayText(BRIGHT_MAGENTA, "No gamecontroller found...\n");
		}
	}
}