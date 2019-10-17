/*
***********************************************
*                                             *
* HID_Keyboard.hpp                            *
*                                             *
* (c) 2017, 2018, 2019 Stefan Kubsch          *
***********************************************
*/

#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>
#include <map>

#include "Tools_ErrorHandling.hpp"

namespace HID_Keyboard
{


	void Init();
	bool WaitForKeypress(std::int_fast32_t Key);
	bool GetKeyState(std::int_fast32_t Key);
	void SetKeyState(std::int_fast32_t Key, bool State);

	//
	// Variables and constants
	//

	inline std::map<std::int_fast32_t, bool> KeyMap{};

	// for Microsoft virtual keycodes, have a look here:
	// https://docs.microsoft.com/en-us/uwp/api/windows.system.virtualkey
	// Values are decimal

	// Default values are:
	//
	// Forward				= W			= 87
	// Backward				= S			= 83
	// Strafe left			= A			= 65
	// Strafe right			= D			= 68
	// Reload weapon		= R			= 82
	// HUD					= H			= 72
	// Minimap				= M			= 77
	// Action/Open door		= Space		= 32
	// Increase mouse		= +			= 107
	// Decrease mouse		= -			= 109
	// Next level			= N			= 78
	// Switch lighting		= L			= 76
	// Pause/menu			= ESC		= 27
	// Menu item down		= VK_DOWN	= 40
	// Menu item up			= VK_UP		= 38
	// Select menu item		= ENTER		= 13

	inline std::int_fast32_t MovePlayerForwardKey{};
	inline std::int_fast32_t MovePlayerBackwardKey{};
	inline std::int_fast32_t MovePlayerStrafeLeftKey{};
	inline std::int_fast32_t MovePlayerStrafeRightKey{};
	inline std::int_fast32_t ReloadWeaponKey{};
	inline std::int_fast32_t HUDKey{};
	inline std::int_fast32_t MiniMapKey{};
	inline std::int_fast32_t ActionKey{};
	inline std::int_fast32_t IncreaseMouseSensitivityKey{};
	inline std::int_fast32_t DecreaseMouseSensitivityKey{};
	inline std::int_fast32_t SelectNextLevelKey{};
	inline std::int_fast32_t SwitchLightingKey{};
	inline std::int_fast32_t PauseKey{};
	inline std::int_fast32_t MenuItemDownKey{};
	inline std::int_fast32_t MenuItemUpKey{};
	inline std::int_fast32_t MenuItemSelectKey{};

	//
	// Functions
	//

	inline void Init()
	{
		if (const std::string INIFile{ GameConfigFolder + "InputConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			MovePlayerForwardKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "MoveForwardKey");
			MovePlayerBackwardKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "MoveBackwardKey");
			MovePlayerStrafeLeftKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "StrafeLeftKey");
			MovePlayerStrafeRightKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "StrafeRightKey");
			ReloadWeaponKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "ReloadWeaponKey");
			HUDKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "HUDKey");
			MiniMapKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "MiniMapKey");
			ActionKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "ActionKey");
			IncreaseMouseSensitivityKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "IncreaseMouseSensitivityKey");
			DecreaseMouseSensitivityKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "DecreaseMouseSensitivityKey");
			SelectNextLevelKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "SelectNextLevelKey");
			SwitchLightingKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "SwitchLightingKey");
			PauseKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "PauseKey");
			MenuItemDownKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "MenuItemDownKey");
			MenuItemUpKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "MenuItemUpKey");
			MenuItemSelectKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "KEYBOARD", "MenuItemSelectKey");

			lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::DeviceIdentifier::HID_KEYBOARD);
		}
	}

	inline bool WaitForKeypress(const std::int_fast32_t Key)
	{
		while (true)
		{
			if (GetAsyncKeyState(Key) != 0)
			{
				break;
			}
		}

		return true;
	}

	inline bool GetKeyState(const std::int_fast32_t Key)
	{
		const auto State{ KeyMap.find(Key) };

		if (State == KeyMap.end())
		{
			return false;
		}

		return State->second;
	}

	inline void SetKeyState(const std::int_fast32_t Key, const bool State)
	{
		KeyMap[Key] = State;
	}


} // namespace HID_Keyboard