/*
*******************************************************
*                                                     *
* lwmf_gamepad - lightweight media framework          *
*                                                     *
* (C) 2019 - present by Stefan Kubsch                 *
*                                                     *
*******************************************************
*/

#pragma once

#include <Windows.h>
#include <Xinput.h>
#include <cstdint>
#include <algorithm>
#include <string>
#include <map>
#include <cmath>

#pragma comment(lib, "xinput")

namespace lwmf
{


	class Gamepad
	{
	public:
		enum class AnalogButtons : std::int_fast32_t
		{
			LeftStickLeft,
			LeftStickRight,
			LeftStickUp,
			LeftStickDown,
			RightStickLeft,
			RightStickRight,
			RightStickUp,
			RightStickDown,
			LeftTrigger,
			RightTrigger,
			Counter
		};

		struct AnalogMapping
		{
			std::int_fast32_t Key{};
			float Threshold{};
		};

		Gamepad();
		void SetButtons();
		void SetDeadzone(float x, float y);
		bool CheckConnection();
		void Refresh();
		void AddKeyMapping(WORD Button, std::int_fast32_t Key);
		void RemoveKeyMappingByButton(WORD Button);
		void RemoveKeyMapping(std::int_fast32_t Key);
		void AddAnalogKeyMapping(const AnalogButtons& Button, float Threshold, std::int_fast32_t Key);
		void RemoveAnalogKeyMapping(AnalogButtons Button);
		void DeleteMappings();
		void SetIntervalAll(std::uint_fast32_t Time);
		void SetInterval(WORD Button, std::uint_fast32_t Time);
		void SetAnalogInterval(const AnalogButtons& Button, std::uint_fast32_t Time);

		std::map<WORD, std::string> Buttons;
		lwmf::IntPointStruct RightStickPos;
		lwmf::FloatPointStruct LeftStick{};
		lwmf::FloatPointStruct RightStick{};
		std::int_fast32_t ControllerID{ -1 };
		float TriggerLeft{};
		float TriggerRight{};
		float RotationXLimit{ 0.01F };
		float Sensitivity{ 0.3F };

	private:
		void SendAnalogKeys(const AnalogButtons& Button, float Now, float Before, float Threshold, std::int_fast32_t Key);

		XINPUT_STATE State{};
		XINPUT_STATE Previous{};

		std::map<WORD, std::int_fast32_t> KeyMap;
		std::map<AnalogButtons, AnalogMapping> AnalogMap;
		std::map<WORD, std::uint32_t> Repeat;
		std::map<AnalogButtons, std::uint32_t> AnalogRepeat;
		std::map<WORD, DWORD> LastPress;
		std::map<AnalogButtons, DWORD> AnalogLastPress;

		lwmf::FloatPointStruct DeadZone{ 0.3F, 0.3F };
		lwmf::FloatPointStruct PreviousLeftStick{};
		lwmf::FloatPointStruct PreviousRightStick{};
		float PreviousLeftTrigger{};
		float PreviousRightTrigger{};
	};

	inline Gamepad::Gamepad()
	{
		SetButtons();
	}

	inline void Gamepad::SetButtons()
	{
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_A, "A"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_B, "B"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_X, "X"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_Y, "Y"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_DPAD_LEFT, "DPLeft"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_DPAD_RIGHT, "DPRight"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_DPAD_UP, "DPUp"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_DPAD_DOWN, "DPDown"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_LEFT_SHOULDER, "LSB"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_RIGHT_SHOULDER, "RSB"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_BACK, "Back"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_START, "Start"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_LEFT_THUMB, "LT"));
		Buttons.insert(std::make_pair<WORD, std::string>(XINPUT_GAMEPAD_RIGHT_THUMB, "RT"));
	}

	inline void Gamepad::SetDeadzone(const float x, const float y)
	{
		DeadZone = { x, y };
	}

	inline bool Gamepad::CheckConnection()
	{
		for (std::int_fast32_t i{}; i < XUSER_MAX_COUNT && ControllerID == -1; ++i)
		{
			XINPUT_STATE XInputState;
			ZeroMemory(&XInputState, sizeof(XINPUT_STATE));

			if (XInputGetState(i, &XInputState) == ERROR_SUCCESS)
			{
				ControllerID = i;
				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "XBOX controller found...");
			}
			else
			{
				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "No XBOX controller found...");
			}
		}

		return ControllerID != -1;
	}

	inline void Gamepad::Refresh()
	{
		Previous = State;
		PreviousLeftStick = LeftStick;
		PreviousRightStick = RightStick;
		PreviousLeftTrigger = TriggerLeft;
		PreviousRightTrigger = TriggerRight;

		ZeroMemory(&State, sizeof(XINPUT_STATE));
		XInputGetState(ControllerID, &State);

		const float NormalizedLX{ max(-1, static_cast<float>(State.Gamepad.sThumbLX) / SHRT_MAX) };
		const float NormalizedLY{ max(-1, static_cast<float>(State.Gamepad.sThumbLY) / SHRT_MAX) };

		LeftStick.X = (std::abs(NormalizedLX) < DeadZone.X ? 0.0F : (std::abs(NormalizedLX) - DeadZone.X) * (NormalizedLX / std::abs(NormalizedLX)));
		LeftStick.Y = (std::abs(NormalizedLY) < DeadZone.Y ? 0.0F : (std::abs(NormalizedLY) - DeadZone.Y) * (NormalizedLY / std::abs(NormalizedLY)));

		if (DeadZone.X > 0.0F)
		{
			LeftStick.X *= 1.0F / (1.0F - DeadZone.X);
		}

		if (DeadZone.Y > 0.0F)
		{
			LeftStick.Y *= 1.0F / (1.0F - DeadZone.Y);
		}

		const float NormalizedRX{ max(-1, static_cast<float>(State.Gamepad.sThumbRX) / SHRT_MAX) };
		const float NormalizedRY{ max(-1, static_cast<float>(State.Gamepad.sThumbRY) / SHRT_MAX) };

		RightStick.X = (std::abs(NormalizedRX) < DeadZone.X ? 0.0F : (std::abs(NormalizedRX) - DeadZone.X) * (NormalizedRX / std::abs(NormalizedRX)));
		RightStick.Y = (std::abs(NormalizedRY) < DeadZone.Y ? 0.0F : (std::abs(NormalizedRY) - DeadZone.Y) * (NormalizedRY / std::abs(NormalizedRY)));

		if (DeadZone.X > 0.0F)
		{
			RightStick.X *= 1.0F / (1.0F - DeadZone.X);
		}

		if (DeadZone.Y > 0.0F)
		{
			RightStick.Y *= 1.0F / (1.0F - DeadZone.Y);
		}

		TriggerLeft = static_cast<float>(State.Gamepad.bLeftTrigger) / 255;
		TriggerRight = static_cast<float>(State.Gamepad.bRightTrigger) / 255;

		for (const auto& Button : Buttons)
		{
			if ((State.Gamepad.wButtons & Button.first) != 0)
			{
				const WORD Mapping{ (KeyMap.find(Button.first) != KeyMap.end() ? static_cast<WORD>(KeyMap.find(Button.first)->second) : static_cast<WORD>(Button.first)) };
				const DWORD Now{ GetTickCount() };
				const DWORD Last{ (LastPress.find(Button.first) != LastPress.end() ? LastPress.find(Button.first)->second : 0) };
				const std::uint_fast32_t Time{ Repeat.find(Button.first)->second };

				if ((Now - Last >= Time && Time > 0) || Last == 0 || (Time == 0 && (Previous.Gamepad.wButtons & Button.first) == 0))
				{
					LastPress.erase(Button.first);
					LastPress.insert(std::map<WORD, DWORD>::value_type(Button.first, Now));

					SendMessage(MainWindow, WM_KEYDOWN, Mapping, ((Previous.Gamepad.wButtons & Button.first) == 0 ? 0 << 30 : 1 << 30));
				}
			}

			if (Previous.dwPacketNumber < State.dwPacketNumber)
			{
				if ((State.Gamepad.wButtons & Button.first) == 0 && (Previous.Gamepad.wButtons & Button.first) != 0)
				{
					LastPress.erase(Button.first);
					SendMessage(MainWindow, WM_KEYUP, (KeyMap.find(Button.first) != KeyMap.end() ? static_cast<WORD>(KeyMap.find(Button.first)->second) : static_cast<WORD>(Button.first)), 0);
				}
			}
		}

		for (const auto& Item : AnalogMap)
		{
			const WORD Mapping{ static_cast<WORD>(Item.second.Key) };

			switch (Item.first)
			{
				case AnalogButtons::LeftStickLeft:
				{
					SendAnalogKeys(AnalogButtons::LeftStickLeft, LeftStick.X, PreviousLeftStick.X, -Item.second.Threshold, Mapping);
					break;
				}
				case AnalogButtons::LeftStickRight:
				{
					SendAnalogKeys(AnalogButtons::LeftStickRight, LeftStick.X, PreviousLeftStick.X, Item.second.Threshold, Mapping);
					break;
				}
				case AnalogButtons::LeftStickUp:
				{
					SendAnalogKeys(AnalogButtons::LeftStickUp, LeftStick.Y, PreviousLeftStick.Y, Item.second.Threshold, Mapping);
					break;
				}
				case AnalogButtons::LeftStickDown:
				{
					SendAnalogKeys(AnalogButtons::LeftStickDown, LeftStick.Y, PreviousLeftStick.Y, -Item.second.Threshold, Mapping);
					break;
				}
				case AnalogButtons::RightStickLeft:
				{
					SendAnalogKeys(AnalogButtons::RightStickLeft, RightStick.X, PreviousRightStick.X, -Item.second.Threshold, Mapping);
					break;
				}
				case AnalogButtons::RightStickRight:
				{
					SendAnalogKeys(AnalogButtons::RightStickRight, RightStick.X, PreviousRightStick.X, Item.second.Threshold, Mapping);
					break;
				}
				case AnalogButtons::RightStickUp:
				{
					SendAnalogKeys(AnalogButtons::RightStickUp, RightStick.Y, PreviousRightStick.Y, Item.second.Threshold, Mapping);
					break;
				}
				case AnalogButtons::RightStickDown:
				{
					SendAnalogKeys(AnalogButtons::RightStickDown, RightStick.Y, PreviousRightStick.Y, -Item.second.Threshold, Mapping);
					break;
				}
				case AnalogButtons::LeftTrigger:
				{
					SendAnalogKeys(AnalogButtons::LeftTrigger, TriggerLeft, PreviousLeftTrigger, Item.second.Threshold, Mapping);
					break;
				}
				case AnalogButtons::RightTrigger:
				{
					SendAnalogKeys(AnalogButtons::RightTrigger, TriggerRight, PreviousRightTrigger, Item.second.Threshold, Mapping);
					break;
				}
				default: {}
			}
		}
	}

	inline void Gamepad::AddKeyMapping(const WORD Button, const std::int_fast32_t Key)
	{
		KeyMap.erase(Button);
		KeyMap.insert(std::map<WORD, std::int_fast32_t>::value_type(Button, Key));
	}

	inline void Gamepad::RemoveKeyMapping(const std::int_fast32_t Key)
	{
		for (auto it{ KeyMap.begin() }; it != KeyMap.end(); ++it)
		{
			if (it->second == Key)
			{
				KeyMap.erase(it);
				break;
			}
		}
	}

	inline void Gamepad::RemoveKeyMappingByButton(const WORD Button)
	{
		KeyMap.erase(Button);
	}

	inline void Gamepad::AddAnalogKeyMapping(const AnalogButtons& Button, const float Threshold, const std::int_fast32_t Key)
	{
		const AnalogMapping AnalogKeyMapping { Key, Threshold };

		AnalogMap.erase(Button);
		AnalogMap.insert(std::make_pair(Button, AnalogKeyMapping));
	}

	inline void Gamepad::RemoveAnalogKeyMapping(AnalogButtons Button)
	{
		AnalogMap.erase(Button);
	}

	inline void Gamepad::DeleteMappings()
	{
		KeyMap.clear();
		AnalogMap.clear();
	}

	inline void Gamepad::SetIntervalAll(const std::uint_fast32_t Time)
	{
		Repeat.clear();

		for (const auto& Button : Buttons)
		{
			Repeat.insert(std::map<WORD, std::uint_fast32_t>::value_type(Button.first, Time));
		}

		AnalogRepeat.clear();

		for (std::int_fast32_t i{}; i < static_cast<std::int_fast32_t>(AnalogButtons::Counter); ++i)
		{
			AnalogRepeat.insert(std::map<AnalogButtons, std::uint_fast32_t>::value_type(static_cast<AnalogButtons>(i), Time));
		}
	}

	inline void Gamepad::SetInterval(const WORD Button, const std::uint_fast32_t Time)
	{
		Repeat.erase(Button);
		Repeat.insert(std::map<WORD, std::uint_fast32_t>::value_type(Button, Time));
	}

	inline void Gamepad::SetAnalogInterval(const AnalogButtons& Button, const std::uint_fast32_t Time)
	{
		AnalogRepeat.erase(Button);
		AnalogRepeat.insert(std::map<AnalogButtons, std::uint_fast32_t>::value_type(Button, Time));
	}

	inline void Gamepad::SendAnalogKeys(const AnalogButtons& Button, const float Now, const float Before, const float Threshold, const std::int_fast32_t Key)
	{
		const bool IsPressed{ (Now >= Threshold && Threshold > 0.0F) || (Now <= Threshold && Threshold < 0.0F) };
		const bool WasPressed{ (Before >= Threshold && Threshold > 0.0F) || (Before <= Threshold && Threshold < 0.0F) };

		if (IsPressed)
		{
			const DWORD TempNow{ GetTickCount() };
			const DWORD TempBefore{ (AnalogLastPress.find(Button) != AnalogLastPress.end() ? AnalogLastPress.find(Button)->second : 0) }; //-V783
			const std::uint_fast32_t Time{ AnalogRepeat.find(Button)->second }; //-V783

			if ((TempNow - TempBefore >= Time && Time > 0) || TempBefore == 0 || (Time == 0 && !WasPressed))
			{
				AnalogLastPress.erase(Button);
				AnalogLastPress.insert(std::map<AnalogButtons, DWORD>::value_type(Button, TempNow));
				SendMessage(MainWindow, WM_KEYDOWN, static_cast<WPARAM>(Key), (WasPressed ? 1 << 30 : 0 << 30));
			}
		}

		if (Previous.dwPacketNumber < State.dwPacketNumber)
		{
			if (!IsPressed && WasPressed)
			{
				AnalogLastPress.erase(Button);

				SendMessage(MainWindow, WM_KEYUP, static_cast<WPARAM>(Key), 0);
			}
		}
	}


} // namespace lwmf