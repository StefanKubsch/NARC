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

#include "Tools_ErrorHandling.hpp"

namespace HID_Keyboard
{


	void Init();
	bool WaitForKeypress(char Key);
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
			MovePlayerForwardKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "MoveForwardKey");
			MovePlayerBackwardKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "MoveBackwardKey");
			MovePlayerStrafeLeftKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "StrafeLeftKey");
			MovePlayerStrafeRightKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "StrafeRightKey");
			ReloadWeaponKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "ReloadWeaponKey");
			HUDKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "HUDKey");
			MiniMapKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "MiniMapKey");
			IncreaseMouseSensitivityKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "IncreaseMouseSensitivityKey");
			DecreaseMouseSensitivityKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "DecreaseMouseSensitivityKey");
			SelectNextLevelKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "SelectNextLevelKey");
			SwitchLightingKey = lwmf::ReadINIValue<std::int_fast8_t>(INIFile, "KEYBOARD", "SwitchLightingKey");

			lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::DeviceIdentifier::HID_KEYBOARD);
		}
	}

	inline bool WaitForKeypress(const char Key)
	{
		bool Result{};

		while (!Result)
		{
			if (GetAsyncKeyState(Key) != 0)
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