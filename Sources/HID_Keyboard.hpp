/*
***********************************************
*                                             *
* HID_Keyboard.hpp                            *
*                                             *
* (c) 2017, 2018, 2019 Stefan Kubsch          *
***********************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <SDL.h>

#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"

namespace HID_Keyboard
{


	void Init();
	bool WaitForKeypress();
	bool GetKeyState(char Key);
	void SetKeyState(char Key, bool State);

	//
	// Variables and constants
	//

	inline std::map<char, bool> KeyMap;

	inline char MovePlayerForwardKey;
	inline char MovePlayerBackwardKey;
	inline char MovePlayerStrafeLeftKey;
	inline char MovePlayerStrafeRightKey;
	inline char ReloadWeaponKey;
	inline char HUDKey;
	inline char MiniMapKey;
	inline char IncreaseMouseSensitivityKey;
	inline char DecreaseMouseSensitivityKey;
	inline char SelectNextLevelKey;
	inline char SwitchLightingKey;

	//
	// Functions
	//

	inline void Init()
	{
		if (const std::string INIFile{ "./DATA/GameConfig/InputConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			MovePlayerForwardKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "MoveForwardKey");
			MovePlayerBackwardKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "MoveBackwardKey");
			MovePlayerStrafeLeftKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "StrafeLeftKey");
			MovePlayerStrafeRightKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "StrafeRightKey");
			ReloadWeaponKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "ReloadWeaponKey");
			HUDKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "HUDKey");
			MiniMapKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "MiniMapKey");
			IncreaseMouseSensitivityKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "IncreaseMouseSensitivityKey");
			DecreaseMouseSensitivityKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "DecreaseMouseSensitivityKey");
			SelectNextLevelKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "SelectNextLevelKey");
			SwitchLightingKey = Tools_INIFile::ReadValue<std::int_fast8_t>(INIFile, "KEYBOARD", "SwitchLightingKey");
		}
	}

	inline bool WaitForKeypress()
	{
		SDL_Event Event;
		bool Result{};

		// Clear events that may be in line...
		SDL_PollEvent(&Event);

		while (!Result)
		{
			if (SDL_WaitEvent(&Event) == 1 && (Event.type == SDL_KEYDOWN || Event.type == SDL_MOUSEBUTTONDOWN || Event.type == SDL_CONTROLLERBUTTONDOWN))
			{
				Result = true;
			}
		}

		return Result;
	}

	inline bool GetKeyState(const char Key)
	{
		const auto State{ KeyMap.find(Key) };

		if (State == KeyMap.end())
		{
			return false;
		}

		return State->second;
	}

	inline void SetKeyState(const char Key, const bool State)
	{
		KeyMap[Key] = State;
	}


} // namespace HID_Keyboard