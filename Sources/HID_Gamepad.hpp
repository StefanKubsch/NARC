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

	constexpr std::int_fast32_t VirtMouseUp{ VK_UP };
	constexpr std::int_fast32_t VirtMouseDown{ VK_DOWN };
	constexpr std::int_fast32_t VirtMouseLeft{ VK_LEFT };
	constexpr std::int_fast32_t VirtMouseRight{ VK_RIGHT };

	//
	// Functions
	//

	inline void Init()
	{
		if (GameController.CheckConnection())
		{
			NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Init XBOX controller...");

			if (const std::string INIFile{ "./DATA/GameConfig/InputConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
			{
				const float DeadZone{ lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "DeadZone") };
				GameController.Sensitivity = lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "Sensitivity");
				GameController.RotationXLimit = lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "RotationXLimit");
				GameController.SetRepeatIntervalMsAll(lwmf::ReadINIValue<std::uint_fast32_t>(INIFile, "GAMECONTROLLER", "RepeatIntervall"));

				GameController.SetDeadzone(DeadZone, DeadZone);
				
				GameController.ClearMappings();
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickLeft, DeadZone, HID_Keyboard::MovePlayerStrafeLeftKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickRight, DeadZone, HID_Keyboard::MovePlayerStrafeRightKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickUp, DeadZone, HID_Keyboard::MovePlayerForwardKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickDown, DeadZone, HID_Keyboard::MovePlayerBackwardKey);

				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickLeft, DeadZone, VirtMouseLeft);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickRight, DeadZone, VirtMouseRight);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickUp, DeadZone, VirtMouseUp);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickDown, DeadZone, VirtMouseDown);
			}
		}
	}


} // namespace HID_Gamepad
