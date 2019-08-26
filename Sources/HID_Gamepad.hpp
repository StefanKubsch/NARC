/*
******************************************
*                                        *
* HID_Gamepad.hpp                        *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include "Game_GlobalDefinitions.hpp"
#include "HID_Keyboard.hpp"

namespace HID_Gamepad
{


	void Init();

	//
	// Variables and constants
	//

	inline lwmf::Gamepad GameController;

	constexpr char VirtMouseUp{ VK_UP };
	constexpr char VirtMouseDown{ VK_DOWN };
	constexpr char VirtMouseLeft{ VK_LEFT };
	constexpr char VirtMouseRight{ VK_RIGHT };
	constexpr char FireSingleShotKey{ ',' };
	constexpr char RapidFireKey{ '.' };
	constexpr char ReloadWeaponKey{ 'R' };
	constexpr char ChangeWeaponUpKey{ ';' };
	constexpr char ChangeWeaponDownKey{ ':' };
	constexpr char ActionKey{ VK_SPACE };

	//
	// Functions
	//

	inline void Init()
	{
		if (GameController.CheckConnection())
		{
			NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Init XBOX controller...");

			GameController.SetButtons();

			if (const std::string INIFile{ "./DATA/GameConfig/InputConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
			{
				const float DeadZone{ lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "DeadZone") };
				GameController.Sensitivity = lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "Sensitivity");
				GameController.RotationXLimit = lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "RotationXLimit");
				GameController.SetIntervalAll(lwmf::ReadINIValue<std::uint_fast32_t>(INIFile, "GAMECONTROLLER", "RepeatIntervall"));

				GameController.SetDeadzone(DeadZone, DeadZone);

				GameController.DeleteMappings();
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickLeft, DeadZone, HID_Keyboard::MovePlayerStrafeLeftKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickRight, DeadZone, HID_Keyboard::MovePlayerStrafeRightKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickUp, DeadZone, HID_Keyboard::MovePlayerForwardKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickDown, DeadZone, HID_Keyboard::MovePlayerBackwardKey);

				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickLeft, DeadZone, VirtMouseLeft);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickRight, DeadZone, VirtMouseRight);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickUp, DeadZone, VirtMouseUp);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickDown, DeadZone, VirtMouseDown);

				GameController.SetInterval(XINPUT_GAMEPAD_RIGHT_SHOULDER, 0);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_RIGHT_SHOULDER, FireSingleShotKey);

				GameController.SetInterval(XINPUT_GAMEPAD_LEFT_SHOULDER, 150);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_LEFT_SHOULDER, RapidFireKey);

				GameController.SetInterval(XINPUT_GAMEPAD_DPAD_RIGHT, 0);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_DPAD_RIGHT, ReloadWeaponKey);

				GameController.SetInterval(XINPUT_GAMEPAD_DPAD_UP, 0);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_DPAD_UP, ChangeWeaponUpKey);

				GameController.SetInterval(XINPUT_GAMEPAD_DPAD_DOWN, 0);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_DPAD_DOWN, ChangeWeaponDownKey);

				GameController.SetInterval(XINPUT_GAMEPAD_X, 0);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_X, ActionKey);
			}
		}
	}


} // namespace HID_Gamepad
