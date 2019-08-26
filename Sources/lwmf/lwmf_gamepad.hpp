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
		enum class AnalogButtons
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
			EndOfButtons
		};

		struct AnalogMapping
		{
			std::int_fast32_t Key{};
			float Threshold{};
		};

		void SetDeadzone(float x, float y);
		bool CheckConnection();
		void Refresh();
		bool IsPressed(WORD Button);
		void AddKeyMapping(WORD Button, std::int_fast32_t Key);
		void RemoveKeyMappingByButton(WORD Button);
		void RemoveKeyMapping(std::int_fast32_t Key);
		void AddAnalogKeyMapping(const AnalogButtons& Button, float Threshold, std::int_fast32_t Key);
		void RemoveAnalogKeyMapping(AnalogButtons Button);
		void ClearMappings();
		void SetRepeatIntervalMsAll(std::uint_fast32_t ms);
		void SetRepeatIntervalMs(WORD Button, std::uint_fast32_t ms);
		void SetAnalogRepeatIntervalMs(const AnalogButtons& Button, std::uint_fast32_t ms);

		std::map<WORD, std::string> Buttons;
		lwmf::IntPointStruct RightStickPos;
		float LeftStickX{};
		float LeftStickY{};
		float RightStickX{};
		float RightStickY{};
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
		std::map<WORD, std::uint32_t> RepeatMs;
		std::map<AnalogButtons, std::uint32_t> AnalogRepeatMs;
		std::map<WORD, DWORD> LastPress;
		std::map<AnalogButtons, DWORD> AnalogLastPress;

		std::int_fast32_t ControllerID{};
		
		float DeadZoneX{ 0.3F };
		float DeadZoneY{ 0.3F };
		float PreviousLeftStickX{};
		float PreviousLeftStickY{};
		float PreviousRightStickX{};
		float PreviousRightStickY{};
		float PreviousLeftTrigger{};
		float PreviousRightTrigger{};
	};

	inline void Gamepad::SetDeadzone(const float x, const float y)
	{
		DeadZoneX = x;
		DeadZoneY = y;
	}

	inline bool Gamepad::CheckConnection()
	{
		std::int_fast32_t ID{ -1 };

		for (DWORD i{}; i < XUSER_MAX_COUNT && ID == -1; i++)
		{
			XINPUT_STATE XInputState;
			ZeroMemory(&XInputState, sizeof(XINPUT_STATE));

			if (XInputGetState(i, &XInputState) == ERROR_SUCCESS)
			{
				ID = i;
				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "XBOX controller found...");
			}
			else
			{
				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "No XBOX controller found...");
			}
		}

		ControllerID = ID;

		return ID != -1;
	}

	inline void Gamepad::Refresh()
	{
		Previous = State;
		PreviousLeftStickX = LeftStickX;
		PreviousLeftStickY = LeftStickY;
		PreviousRightStickX = RightStickX;
		PreviousRightStickY = RightStickY;
		PreviousLeftTrigger = TriggerLeft;
		PreviousRightTrigger = TriggerRight;

		ZeroMemory(&State, sizeof(XINPUT_STATE));
		XInputGetState(ControllerID, &State);

		const float NormalizedLX{ max(-1.0F, static_cast<float>(State.Gamepad.sThumbLX / 32767)) };
		const float NormalizedLY{ max(-1.0F, static_cast<float>(State.Gamepad.sThumbLY / 32767)) };

		LeftStickX = (std::abs(NormalizedLX) < DeadZoneX ? 0.0F : (std::fabsf(NormalizedLX) - DeadZoneX) * (NormalizedLX / std::fabsf(NormalizedLX)));
		LeftStickY = (std::abs(NormalizedLY) < DeadZoneY ? 0.0F : (std::fabsf(NormalizedLY) - DeadZoneY) * (NormalizedLY / std::fabsf(NormalizedLY)));

		if (DeadZoneX > 0)
		{
			LeftStickX *= 1 / (1 - DeadZoneX);
		}

		if (DeadZoneY > 0)
		{
			LeftStickY *= 1 / (1 - DeadZoneY);
		}

		const float NormalizedRX{ max(-1.0F, static_cast<float>(State.Gamepad.sThumbRX / 32767)) };
		const float NormalizedRY{ max(-1.0F, static_cast<float>(State.Gamepad.sThumbRY / 32767)) };

		RightStickX = (std::abs(NormalizedRX) < DeadZoneX ? 0.0F : (std::fabsf(NormalizedRX) - DeadZoneX) * (NormalizedRX / std::fabsf(NormalizedRX)));
		RightStickY = (std::abs(NormalizedRY) < DeadZoneY ? 0.0F : (std::fabsf(NormalizedRY) - DeadZoneY) * (NormalizedRY / std::fabsf(NormalizedRY)));

		if (DeadZoneX > 0.0F)
		{
			RightStickX *= 1.0F / (1.0F - DeadZoneX);
		}

		if (DeadZoneY > 0.0F)
		{
			RightStickY *= 1.0F / (1.0F - DeadZoneY);
		}

		TriggerLeft = static_cast<float>(State.Gamepad.bLeftTrigger / 255);
		TriggerRight = static_cast<float>(State.Gamepad.bRightTrigger / 255);

		if (WindowHandle != nullptr)
		{
			for (const auto& Button : Buttons)
			{
				if ((State.Gamepad.wButtons & Button.first) != 0)
				{
					const WORD Mapping{ (KeyMap.find(Button.first) != KeyMap.end() ? static_cast<WORD>(KeyMap.find(Button.first)->second) : static_cast<WORD>(Button.first)) };
					const DWORD Now{ GetTickCount() };
					const DWORD Last{ (LastPress.find(Button.first) != LastPress.end() ? LastPress.find(Button.first)->second : 0) };
					const std::uint_fast32_t ms{ RepeatMs.find(Button.first)->second };

					if ((Now - Last >= ms && ms > 0) || Last == 0 || (ms == 0 && (Previous.Gamepad.wButtons & Button.first) == 0))
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
						const WORD Mapping{ (KeyMap.find(Button.first) != KeyMap.end() ? static_cast<WORD>(KeyMap.find(Button.first)->second) : static_cast<WORD>(Button.first)) };
						LastPress.erase(Button.first);
						SendMessage(MainWindow, WM_KEYUP, Mapping, 0);
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
						SendAnalogKeys(AnalogButtons::LeftStickLeft, LeftStickX, PreviousLeftStickX, -Item.second.Threshold, Mapping);
						break;
					}
					case AnalogButtons::LeftStickRight:
					{
						SendAnalogKeys(AnalogButtons::LeftStickRight, LeftStickX, PreviousLeftStickX, Item.second.Threshold, Mapping);
						break;
					}
					case AnalogButtons::LeftStickUp:
					{
						SendAnalogKeys(AnalogButtons::LeftStickUp, LeftStickY, PreviousLeftStickY, Item.second.Threshold, Mapping);
						break;
					}
					case AnalogButtons::LeftStickDown:
					{
						SendAnalogKeys(AnalogButtons::LeftStickDown, LeftStickY, PreviousLeftStickY, -Item.second.Threshold, Mapping);
						break;
					}
					case AnalogButtons::RightStickLeft:
					{
						SendAnalogKeys(AnalogButtons::RightStickLeft, RightStickX, PreviousRightStickX, -Item.second.Threshold, Mapping);
						break;
					}
					case AnalogButtons::RightStickRight:
					{
						SendAnalogKeys(AnalogButtons::RightStickRight, RightStickX, PreviousRightStickX, Item.second.Threshold, Mapping);
						break;
					}
					case AnalogButtons::RightStickUp:
					{
						SendAnalogKeys(AnalogButtons::RightStickUp, RightStickY, PreviousRightStickY, Item.second.Threshold, Mapping);
						break;
					}
					case AnalogButtons::RightStickDown:
					{
						SendAnalogKeys(AnalogButtons::RightStickDown, RightStickY, PreviousRightStickY, -Item.second.Threshold, Mapping);
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
	}

	inline bool Gamepad::IsPressed(const WORD Button)
	{
		return (State.Gamepad.wButtons & Button) != 0;
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
		AnalogMapping AnalogKeyMapping { Key, Threshold };

		AnalogMap.erase(Button);
		AnalogMap.insert(std::make_pair(Button, AnalogKeyMapping));
	}

	inline void Gamepad::RemoveAnalogKeyMapping(AnalogButtons Button)
	{
		AnalogMap.erase(Button);
	}

	inline void Gamepad::ClearMappings()
	{
		KeyMap.clear();
		AnalogMap.clear();
	}

	inline void Gamepad::SetRepeatIntervalMsAll(const std::uint_fast32_t ms)
	{
		RepeatMs.clear();

		for (const auto& Button : Buttons)
		{
			RepeatMs.insert(std::map<WORD, std::uint_fast32_t>::value_type(Button.first, ms));
		}

		AnalogRepeatMs.clear();

		for (std::int_fast32_t i{}; i < static_cast<std::int_fast32_t>(AnalogButtons::EndOfButtons); ++i)
		{
			AnalogRepeatMs.insert(std::map<AnalogButtons, std::uint_fast32_t>::value_type(static_cast<AnalogButtons>(i), ms));
		}
	}

	inline void Gamepad::SetRepeatIntervalMs(const WORD Button, const std::uint_fast32_t ms)
	{
		RepeatMs.erase(Button);
		RepeatMs.insert(std::map<WORD, std::uint_fast32_t>::value_type(Button, ms));
	}

	inline void Gamepad::SetAnalogRepeatIntervalMs(const AnalogButtons& Button, const std::uint_fast32_t ms)
	{
		AnalogRepeatMs.erase(Button);
		AnalogRepeatMs.insert(std::map<AnalogButtons, std::uint_fast32_t>::value_type(Button, ms));
	}

	inline void Gamepad::SendAnalogKeys(const AnalogButtons& Button, const float Now, const float Before, const float Threshold, const std::int_fast32_t Key)
	{
		const bool IsPressed{ (Now >= Threshold && Threshold > 0.0F) || (Now <= Threshold && Threshold < 0.0F) };
		const bool WasPressed{ (Before >= Threshold && Threshold > 0.0F) || (Before <= Threshold && Threshold < 0.0F) };

		if (IsPressed)
		{
			const DWORD TempNow{ GetTickCount() };
			const DWORD TempBefore{ (AnalogLastPress.find(Button) != AnalogLastPress.end() ? AnalogLastPress.find(Button)->second : 0) };
			const std::uint_fast32_t ms{ AnalogRepeatMs.find(Button)->second };

			if ((TempNow - TempBefore >= ms && ms > 0) || TempBefore == 0 || (ms == 0 && !WasPressed))
			{
				AnalogLastPress.erase(Button);
				AnalogLastPress.insert(std::map<AnalogButtons, DWORD>::value_type(Button, TempNow));
				SendMessage(MainWindow, WM_KEYDOWN, Key, (WasPressed ? 1 << 30 : 0 << 30));
			}
		}

		if (Previous.dwPacketNumber < State.dwPacketNumber)
		{
			if (!IsPressed && WasPressed)
			{
				AnalogLastPress.erase(Button);

				SendMessage(MainWindow, WM_KEYUP, Key, 0);
			}
		}
	}


} // namespace lwmf