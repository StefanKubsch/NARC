/*
******************************************
*                                        *
* HID_Gamepad.hpp                        *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
******************************************
*/

#pragma once

#include <cstdint>

#include "Tools_ErrorHandling.hpp"
#include "HID_Keyboard.hpp"

namespace HID_Gamepad
{


	void Init();

	//
	// Variables and constants
	//

	inline lwmf::Gamepad GameController{};
	inline lwmf::ShaderClass XBoxControllerIconShader{};

	// for Microsoft virtual keycodes, have a look here:
	// https://docs.microsoft.com/en-us/uwp/api/windows.system.virtualkey
	// Values are decimal

	// Default values are:
	//
	// Virtual mouse up		= VK_UP			= 38
	// Virtual mouse down	= VK_DOWN		= 40
	// Virtual mouse left	= VK_LEFT		= 37
	// Virtual mouse right	= VK_RIGHT		= 39
	// Fire single shot		= Left Control	= 162
	// Rapid fire			= Left Shift	= 160
	// Change weapon up		= VK_PRIOR		= 33 (PageUp)
	// Change weapon down	= VK_NEXT		= 35 (PageDown)

	inline std::int_fast32_t VirtMouseUpKey{};
	inline std::int_fast32_t VirtMouseDownKey{};
	inline std::int_fast32_t VirtMouseLeftKey{};
	inline std::int_fast32_t VirtMouseRightKey{};
	inline std::int_fast32_t FireSingleShotKey{};
	inline std::int_fast32_t RapidFireKey{};
	inline std::int_fast32_t ChangeWeaponUpKey{};
	inline std::int_fast32_t ChangeWeaponDownKey{};

	//
	// Functions
	//

	inline void Init()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Init XBOX controller...");

		if (GameController.CheckConnection())
		{
			std::string INIFile{ GameConfigFolder };
			INIFile += "InputConfig.ini";

			if (Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
			{
				const float DeadZone{ lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "DeadZone") };
				GameController.Sensitivity = lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "Sensitivity");
				GameController.RotationXLimit = lwmf::ReadINIValue<float>(INIFile, "GAMECONTROLLER", "RotationXLimit");
				GameController.SetIntervalAll(lwmf::ReadINIValue<std::uint_fast32_t>(INIFile, "GAMECONTROLLER", "RepeatIntervall"));

				VirtMouseUpKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GAMECONTROLLER", "VirtMouseUpKey");
				VirtMouseDownKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GAMECONTROLLER", "VirtMouseDownKey");
				VirtMouseLeftKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GAMECONTROLLER", "VirtMouseLeftKey");
				VirtMouseRightKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GAMECONTROLLER", "VirtMouseRightKey");
				FireSingleShotKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GAMECONTROLLER", "FireSingleShotKey");
				RapidFireKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GAMECONTROLLER", "RapidFireKey");
				ChangeWeaponUpKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GAMECONTROLLER", "ChangeWeaponUpKey");
				ChangeWeaponDownKey = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GAMECONTROLLER", "ChangeWeaponDownKey");

				GameController.SetDeadzone(DeadZone, DeadZone);

				GameController.DeleteMappings();
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickLeft, DeadZone, HID_Keyboard::MovePlayerStrafeLeftKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickRight, DeadZone, HID_Keyboard::MovePlayerStrafeRightKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickUp, DeadZone, HID_Keyboard::MovePlayerForwardKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::LeftStickDown, DeadZone, HID_Keyboard::MovePlayerBackwardKey);

				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickLeft, DeadZone, VirtMouseLeftKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickRight, DeadZone, VirtMouseRightKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickUp, DeadZone, VirtMouseUpKey);
				GameController.AddAnalogKeyMapping(lwmf::Gamepad::AnalogButtons::RightStickDown, DeadZone, VirtMouseDownKey);

				GameController.SetInterval(XINPUT_GAMEPAD_RIGHT_SHOULDER, 0);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_RIGHT_SHOULDER, FireSingleShotKey);

				GameController.SetInterval(XINPUT_GAMEPAD_LEFT_SHOULDER, 150);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_LEFT_SHOULDER, RapidFireKey);

				GameController.SetInterval(XINPUT_GAMEPAD_DPAD_RIGHT, 0);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_DPAD_RIGHT, HID_Keyboard::ReloadWeaponKey);

				GameController.SetInterval(XINPUT_GAMEPAD_DPAD_UP, 0);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_DPAD_UP, ChangeWeaponUpKey);

				GameController.SetInterval(XINPUT_GAMEPAD_DPAD_DOWN, 0);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_DPAD_DOWN, ChangeWeaponDownKey);

				GameController.SetInterval(XINPUT_GAMEPAD_X, 0);
				GameController.AddKeyMapping(XINPUT_GAMEPAD_X, HID_Keyboard::ActionKey);

				// Load XBoxControllerIcon
				const lwmf::TextureStruct TempXBoxControllerIconTexture{ GFX_ImageHandling::ImportImage(lwmf::ReadINIValue<std::string>(INIFile, "GAMECONTROLLER", "XBoxControllerIcon")) };
				XBoxControllerIconShader.LoadShader("Default", ScreenTexture);
				XBoxControllerIconShader.LoadStaticTextureInGPU(TempXBoxControllerIconTexture, &XBoxControllerIconShader.OGLTextureID, ScreenTexture.Width - 153, 0, TempXBoxControllerIconTexture.Width, TempXBoxControllerIconTexture.Height);
			}
		}
	}


} // namespace HID_Gamepad
